#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "esp_ble_mesh_defs.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_ble_mesh_time_scene_model_api.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"

#include "ble_mesh_example_nvs.h"

#include "app/ble_mesh/coexist.h"
#include "app/ble_mesh/time_scene_server.h"
#include "app/ble_mesh/defs.h"

/*
 * Max message sizes are calculated from message descriptions in section 5.2.1
 * of the BLE Mesh spec, v1.1.1.
 */
#define MAX_TIME_SERVER_MSG_SIZE 11
#define MAX_TIME_SETUP_SERVER_MSG_SIZE 11

#define TAI_UTC_OFFSET_SECONDS 946684768
#define TAI_DELTA_ZERO 0x00FF
#define TIME_ZONE_OFFSET_ZERO 0x40

static const char *TAG = "app/ble_mesh/time_scene_server.c";
static const char *NVS_KEY = "ble_mesh_time";

static int64_t last_status_publish_ts = 0;

static esp_ble_mesh_time_state_t time_state = {
  .time.tai_utc_delta_curr = TAI_DELTA_ZERO + 37,
  .time.tai_utc_delta_new = TAI_DELTA_ZERO + 37,
  .time.time_zone_offset_curr = TIME_ZONE_OFFSET_ZERO,
  .time.time_zone_offset_new = TIME_ZONE_OFFSET_ZERO,
  .time_role = ESP_BLE_MESH_TIME_NONE,
};

static app_time_persist_state_t persist_state = {
  .time_zone_offset_curr = TIME_ZONE_OFFSET_ZERO,
  .time_zone_offset_new = TIME_ZONE_OFFSET_ZERO,
  .tai_utc_delta_curr = TAI_DELTA_ZERO + 37,
  .tai_utc_delta_new = TAI_DELTA_ZERO + 37,
  .time_role = ESP_BLE_MESH_TIME_NONE,
};

static esp_timer_handle_t tz_update_timer;

extern nvs_handle_t APP_BLE_MESH_NVS_HANDLE;

APP_BLE_MESH_MODEL_PUB_DEFINE(time_server_pub, MAX_TIME_SERVER_MSG_SIZE, ROLE_NODE);
esp_ble_mesh_time_srv_t time_server = {
  .rsp_ctrl = {
    .get_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP,
    .set_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP,
    .status_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP,
  },
  .state = &time_state,
};

APP_BLE_MESH_MODEL_PUB_DEFINE(time_setup_pub, MAX_TIME_SETUP_SERVER_MSG_SIZE, ROLE_NODE);
esp_ble_mesh_time_setup_srv_t time_setup_server = {
  .rsp_ctrl = {
    .get_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP,
    .set_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP,
    .status_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP,
  },
  .state = &time_state,
};

static inline struct timeval ble_mesh_time_to_timeval(const uint8_t *tai_seconds, const uint8_t subsecond, const uint8_t tai_utc_delta_curr) {
  // convert ble mesh tai seconds to epoch seconds
  time_t epoch_seconds = (time_t)TAI_UTC_OFFSET_SECONDS;
  epoch_seconds += (time_t) tai_seconds[0]       |
                   (time_t) tai_seconds[1] <<  8 |
                   (time_t) tai_seconds[2] << 16 |
                   (time_t) tai_seconds[3] << 24 |
                   (time_t) tai_seconds[4] << 32;
  // apply tai delta
  // epoch_seconds += (tai_utc_delta_curr - TAI_DELTA_ZERO);
  struct timeval res = {
    .tv_sec = epoch_seconds,
    .tv_usec = (suseconds_t) subsecond << 8,
  };
  return res;
}

static inline struct tm ble_mesh_time_to_tm(const uint8_t *tai_seconds, const uint8_t subsecond, const uint8_t tai_utc_delta_curr, const uint8_t time_zone_offset_curr) {
  struct timeval t = ble_mesh_time_to_timeval(tai_seconds, subsecond, tai_utc_delta_curr);
  struct tm res = {0};
  gmtime_r(&t.tv_sec, &res);
  return res;
}

static inline int64_t time_since_last_publish() {
  // TODO handle timer reset due to deep sleep
  return last_status_publish_ts == 0 ? INT64_MAX : esp_timer_get_time() - last_status_publish_ts;
}

static bool set_timezone(uint8_t timezone, const uint8_t *tai_zone_change, bool persist) {
  // cancel any previously scheduled timezone update
  // error checking not required here.
  esp_timer_stop(tz_update_timer);

  // update or schedule
  time_state.time.time_zone_offset_new = timezone;

  time_t delay = 0;
  struct timeval schedule;
  if (tai_zone_change != NULL) {
    if (!(
      tai_zone_change[0] == 0x00 &&
      tai_zone_change[1] == 0x00 &&
      tai_zone_change[2] == 0x00 &&
      tai_zone_change[3] == 0x00 &&
      tai_zone_change[4] == 0x00
    )) {
      schedule = ble_mesh_time_to_timeval(tai_zone_change, 0, time_state.time.tai_utc_delta_curr);
      struct timeval now;
      gettimeofday(&now, NULL);
      delay = schedule.tv_sec - now.tv_sec;
    }
  }

  if (delay > 0) {
    memcpy(time_state.time.tai_zone_change, tai_zone_change,  sizeof(time_state.time.tai_zone_change));
  } else {
    time_state.time.time_zone_offset_curr = timezone;
    memset(time_state.time.tai_zone_change, 0x00, sizeof(time_state.time.tai_zone_change));
  }

  if (persist) {
    persist_state.time_zone_offset_new = timezone;
    persist_state.time_zone_offset_curr = time_state.time.time_zone_offset_curr;
    if (delay > 0) {
      memcpy(persist_state.tai_zone_change, tai_zone_change,  sizeof(persist_state.tai_zone_change));
    } else {
      persist_state.time_zone_offset_curr = timezone;
      memset(persist_state.tai_zone_change, 0x00, sizeof(persist_state.tai_zone_change));
    }
    ESP_ERROR_CHECK_WITHOUT_ABORT(ble_mesh_nvs_store(APP_BLE_MESH_NVS_HANDLE, NVS_KEY, &persist_state, sizeof(persist_state)));
  }

  bool result = false;
  if (delay > 0) {
    ESP_LOGI(TAG, "scheduling timezone update");
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_timer_start_once(tz_update_timer, (uint64_t) delay * 1000000));
  } else {
    ESP_LOGI(TAG, "setting timezone");
    char tz_str[14];
    int16_t tz_offset = ((int16_t) timezone - TIME_ZONE_OFFSET_ZERO) * 15;
    int16_t tz_hrs = tz_offset / 60;
    int16_t tz_mins = tz_offset < 0 ? -1 * tz_offset % 60 : tz_offset % 60;
    if (sprintf(tz_str, "<%+03d%02d>%d:%02d", tz_hrs, tz_mins, -1 * tz_hrs, tz_mins) >= 0) {
      if (setenv("TZ", tz_str, 1) == 0) {
        tzset();
        ESP_LOGI(TAG, "time zone updated to %s (%s)", tz_str, tzname[0]);
        result = true;
      } else {
        ESP_LOGW(TAG, "failed to set TZ=%s", tz_str);
      }
    } else {
      ESP_LOGW(TAG, "failed to set timezone string: %+02d:%02d: %d", tz_hrs, tz_mins, errno);
    }
  }
  return result;
}

static bool set_time(uint8_t *tai_seconds, uint8_t subsecond, uint8_t uncertainty, uint16_t time_authority, uint16_t tai_utc_delta_curr, uint8_t time_zone_offset_curr) {
  bool res = false;
  if (!(
    tai_seconds[0] == 0x00 &&
    tai_seconds[1] == 0x00 &&
    tai_seconds[2] == 0x00 &&
    tai_seconds[3] == 0x00 &&
    tai_seconds[4] == 0x00
  )) {
    bool has_pending_tz_update = esp_timer_is_active(tz_update_timer);
    char err_str[6];
    struct timeval t = ble_mesh_time_to_timeval(tai_seconds, subsecond, tai_utc_delta_curr);
    if (settimeofday(&t, NULL) != 0) {
      strerror_r(errno, err_str, sizeof(err_str));
      ESP_LOGE(TAG, "failed to set system time: %s", err_str);
      return false;
    }
    if (has_pending_tz_update) {
      set_timezone(time_state.time.time_zone_offset_new, time_state.time.tai_zone_change, true);
    }

    res = true;
    struct tm t1 = {0};
    localtime_r(&t.tv_sec, &t1);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%F %T%z", &t1);
    ESP_LOGI(TAG, "time set: %s", time_str);
  }

  memcpy(time_state.time.tai_seconds, tai_seconds, 5);
  time_state.time.subsecond = subsecond;
  time_state.time.uncertainty = uncertainty;
  time_state.time.time_authority = time_authority;
  time_state.time.tai_utc_delta_curr = tai_utc_delta_curr;
  time_state.time.time_zone_offset_curr = time_zone_offset_curr;

  persist_state.time_zone_offset_curr = time_zone_offset_curr;
  persist_state.tai_utc_delta_curr = tai_utc_delta_curr;
  persist_state.uncertainty = uncertainty;
  persist_state.time_authority = time_authority;
  ESP_ERROR_CHECK_WITHOUT_ABORT(ble_mesh_nvs_store(APP_BLE_MESH_NVS_HANDLE, NVS_KEY, &persist_state, sizeof(persist_state)));
  return res;
}

static void tz_update_timer_cb(void *args) {
  ESP_LOGI(TAG, "updating timezone from timer schedule");
  set_timezone(time_state.time.time_zone_offset_new, NULL, true);
}

static inline void populate_time_status(esp_ble_mesh_time_status_cb_t *time_status) {
  struct timeval tm = {0};
  char err_str[6];
  if (gettimeofday(&tm, NULL) != 0) {
    strerror_r(errno, err_str, sizeof(err_str));
    ESP_LOGE(TAG, "failed to get current system time for time status: %s", err_str);
    return;
  }

  tm.tv_sec -= TAI_UTC_OFFSET_SECONDS;
  time_status->tai_seconds[0] = (uint8_t) tm.tv_sec;
  time_status->tai_seconds[1] = (uint8_t) (tm.tv_sec >> 8);
  time_status->tai_seconds[2] = (uint8_t) (tm.tv_sec >> 16);
  time_status->tai_seconds[3] = (uint8_t) (tm.tv_sec >> 24);
  time_status->tai_seconds[4] = (uint8_t) (tm.tv_sec >> 32);
  time_status->sub_second = (uint8_t) (tm.tv_usec >> 8);
  time_status->time_authority = time_state.time.time_authority;
  time_status->uncertainty = time_state.time.uncertainty + 3;
  time_status->time_zone_offset = time_state.time.time_zone_offset_curr;
  time_status->tai_utc_delta = time_state.time.tai_utc_delta_curr;
}

static void handle_time_msg(esp_ble_mesh_model_t *model,
  esp_ble_mesh_msg_ctx_t *ctx,
  esp_ble_mesh_time_scene_server_cb_value_t *value) {
    esp_ble_mesh_time_status_cb_t time_status = {0};
    esp_ble_mesh_time_zone_status_cb_t tz_status_msg = {0};
    esp_ble_mesh_tai_utc_delta_status_cb_t tai_delta_status_msg = {0};
    esp_ble_mesh_time_role_status_cb_t time_role_status_msg = {0};
    switch(ctx->recv_op) {
      case ESP_BLE_MESH_MODEL_OP_TIME_SET:
        set_time(value->set.time.tai_seconds,
          value->set.time.subsecond,
          value->set.time.uncertainty,
          value->set.time.time_authority,
          value->set.time.tai_utc_delta,
          value->set.time.time_zone_offset);
        __attribute__ ((fallthrough));
      case ESP_BLE_MESH_MODEL_OP_TIME_GET:
        populate_time_status(&time_status);
        COEX_BLE_TRAFFIC(
          esp_ble_mesh_server_model_send_msg(model, ctx, ESP_BLE_MESH_MODEL_OP_TIME_STATUS, sizeof(time_status), (uint8_t *) &time_status)
        );
        break;
      case ESP_BLE_MESH_MODEL_OP_TIME_ZONE_SET:
        set_timezone(value->set.time_zone.time_zone_offset_new, value->set.time_zone.tai_zone_change, true);
        __attribute__ ((fallthrough));
      case ESP_BLE_MESH_MODEL_OP_TIME_ZONE_GET:
        tz_status_msg.time_zone_offset_curr = time_state.time.time_zone_offset_curr;
        tz_status_msg.time_zone_offset_new = time_state.time.time_zone_offset_new;
        memcpy(tz_status_msg.tai_zone_change, &time_state.time.tai_zone_change,  sizeof(tz_status_msg.tai_zone_change));
        COEX_BLE_TRAFFIC(
          esp_ble_mesh_server_model_send_msg(model, ctx, ESP_BLE_MESH_MODEL_OP_TIME_ZONE_STATUS, sizeof(tz_status_msg), (uint8_t *) &tz_status_msg)
        );
        break;
      case ESP_BLE_MESH_MODEL_OP_TAI_UTC_DELTA_SET:
        time_state.time.tai_utc_delta_curr = value->set.tai_utc_delta.tai_utc_delta_new;
        // TODO handle scheduled tai change
        time_state.time.tai_utc_delta_new = value->set.tai_utc_delta.tai_utc_delta_new;
        memcpy(time_state.time.tai_delta_change, value->set.tai_utc_delta.tai_delta_change, sizeof(time_state.time.tai_delta_change));

        persist_state.tai_utc_delta_curr = value->set.tai_utc_delta.tai_utc_delta_new;
        persist_state.tai_utc_delta_new = value->set.tai_utc_delta.tai_utc_delta_new;
        memcpy(persist_state.tai_delta_change, value->set.tai_utc_delta.tai_delta_change, sizeof(persist_state.tai_delta_change));
        ESP_ERROR_CHECK_WITHOUT_ABORT(ble_mesh_nvs_store(APP_BLE_MESH_NVS_HANDLE, NVS_KEY, &persist_state, sizeof(persist_state)));
        __attribute__ ((fallthrough));
      case ESP_BLE_MESH_MODEL_OP_TAI_UTC_DELTA_GET:
        memcpy(tai_delta_status_msg.tai_delta_change, time_state.time.tai_zone_change, sizeof(tai_delta_status_msg.tai_delta_change));
        tai_delta_status_msg.tai_utc_delta_curr = time_state.time.tai_utc_delta_curr;
        tai_delta_status_msg.tai_utc_delta_new = time_state.time.tai_utc_delta_new;
        tai_delta_status_msg.padding_1 = 0b0;
        tai_delta_status_msg.padding_2 = 0b0;
        COEX_BLE_TRAFFIC(
          esp_ble_mesh_server_model_send_msg(model, ctx, ESP_BLE_MESH_MODEL_OP_TAI_UTC_DELTA_STATUS, sizeof(tai_delta_status_msg), (uint8_t *) &tai_delta_status_msg)
        );
        break;
      case ESP_BLE_MESH_MODEL_OP_TIME_ROLE_SET:
        time_state.time_role = value->set.time_role.time_role;
        persist_state.time_role = value->set.time_role.time_role;
        ESP_ERROR_CHECK_WITHOUT_ABORT(ble_mesh_nvs_store(APP_BLE_MESH_NVS_HANDLE, NVS_KEY, &persist_state, sizeof(persist_state)));
        __attribute__ ((fallthrough));
      case ESP_BLE_MESH_MODEL_OP_TIME_ROLE_GET:
        time_role_status_msg.time_role = time_state.time_role;
        COEX_BLE_TRAFFIC(
          esp_ble_mesh_server_model_send_msg(model, ctx, ESP_BLE_MESH_MODEL_OP_TIME_ROLE_STATUS, sizeof(time_role_status_msg), (uint8_t *) &time_role_status_msg)
        );
        break;
      case ESP_BLE_MESH_MODEL_OP_TIME_STATUS:
        if (time_state.time_role == ESP_BLE_MESH_TIME_RELAY ||
          time_state.time_role == ESP_BLE_MESH_TIME_CLIENT) {
            if (set_time(value->status.time_status.tai_seconds,
              value->status.time_status.subsecond,
              value->status.time_status.uncertainty,
              value->status.time_status.time_authority,
              value->status.time_status.tai_utc_delta,
              value->status.time_status.time_zone_offset)
            ) {
              ESP_LOGI(TAG, "time updated from time status message");
              if (
                time_state.time_role == ESP_BLE_MESH_TIME_RELAY &&
                time_server.model->pub->publish_addr != ESP_BLE_MESH_ADDR_UNASSIGNED &&
                time_since_last_publish() > 30000000
              ) {
                ESP_LOGI(TAG, "time relay re-publishing time status");
                // random delay 20-50ms as per ble mesh spec, section 5.3.1.2.3
                uint32_t delay_ms = 20 + (esp_random() % 30);
                vTaskDelay(pdMS_TO_TICKS(delay_ms));

                // send time status
                populate_time_status(&time_status);
                uint8_t prev_ttl = model->pub->ttl;
                model->pub->ttl = 0;
                esp_ble_mesh_model_publish(model, ESP_BLE_MESH_MODEL_OP_TIME_STATUS, sizeof(time_status), (uint8_t *) &time_status, ROLE_NODE);
                model->pub->ttl = prev_ttl;

                // update last publish time
                last_status_publish_ts = esp_timer_get_time();
              }
            }
        } else {
          ESP_LOGI(TAG, "ignoring time status message because time role is 0x%02x", time_state.time_role);
        }
        break;
      default:
        ESP_LOGW(TAG, "unhandled time opcode 0x%04" PRIx32, ctx->recv_op);
    }
  }

void ble_mesh_time_scene_server_cb(esp_ble_mesh_time_scene_server_cb_event_t event, esp_ble_mesh_time_scene_server_cb_param_t *param) {
  ESP_LOGD(TAG, "event 0x%02x, opcode 0x%04" PRIx32 ", src 0x%04x, dst 0x%04x",
      event, param->ctx.recv_op, param->ctx.addr, param->ctx.recv_dst);
  switch(event) {
    case ESP_BLE_MESH_TIME_SCENE_SERVER_STATE_CHANGE_EVT:
      ESP_LOGI(TAG, "ESP_BLE_MESH_TIME_SCENE_SERVER_STATE_CHANGE_EVT");
      if (param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_TIME_SET) {
        set_time(param->value.state_change.time_set.tai_seconds,
          param->value.state_change.time_set.subsecond,
          param->value.state_change.time_set.uncertainty,
          param->value.state_change.time_set.time_authority,
          param->value.state_change.time_set.tai_utc_delta_curr,
          param->value.state_change.time_set.time_zone_offset_curr);
      } else {
        ESP_LOGW(TAG, "time server state change event received for unknown opcode 0x%04" PRIx32, param->ctx.recv_op);
      }
      break;
    case ESP_BLE_MESH_TIME_SCENE_SERVER_RECV_GET_MSG_EVT:
      ESP_LOGI(TAG, "ESP_BLE_MESH_TIME_SCENE_SERVER_RECV_GET_MSG_EVT");
      if (param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_TAI_UTC_DELTA_GET ||
          param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_TIME_GET          ||
          param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_TIME_ROLE_GET     ||
          param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_TIME_ZONE_GET
      ) {
        handle_time_msg(param->model, &param->ctx, &param->value);
      } else {
        ESP_LOGW(TAG, "time server get event received for unknown opcode 0x%04" PRIx32, param->ctx.recv_op);
      }
      break;
    case ESP_BLE_MESH_TIME_SCENE_SERVER_RECV_SET_MSG_EVT:
      ESP_LOGI(TAG, "ESP_BLE_MESH_TIME_SCENE_SERVER_RECV_SET_MSG_EVT");
      if (param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_TIME_SET          ||
          param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_TIME_ZONE_SET     ||
          param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_TAI_UTC_DELTA_SET ||
          param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_TIME_ROLE_SET
      ) {
        handle_time_msg(param->model, &param->ctx, &param->value);
      } else {
        ESP_LOGW(TAG, "time server set event received for unknown opcode 0x%04" PRIx32, param->ctx.recv_op);
      }
      break;
    case ESP_BLE_MESH_TIME_SCENE_SERVER_RECV_STATUS_MSG_EVT:
      ESP_LOGI(TAG, "ESP_BLE_MESH_TIME_SCENE_SERVER_RECV_STATUS_MSG_EVT");
      if (param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_TIME_STATUS) {
        handle_time_msg(param->model, &param->ctx, &param->value);
      } else {
        ESP_LOGW(TAG, "time server status event received for unknown opcode 0x%04" PRIx32, param->ctx.recv_op);
      }
      break;
    default:
      ESP_LOGE(TAG, "Unknown time scene server event 0x%02x", event);
      break;
  }
}

void oob_time_updated(void) {
  ESP_LOGI(TAG, "time updated from oob source");
  time_state.time.uncertainty = 0; // we're certain
  time_state.time.time_authority = 1;
  if (
    (time_state.time_role == ESP_BLE_MESH_TIME_RELAY || time_state.time_role == ESP_BLE_MESH_TIME_AUTHORITY) &&
    time_server.model->pub->publish_addr != ESP_BLE_MESH_ADDR_UNASSIGNED &&
    time_since_last_publish() > 30000000
  ) {
    ESP_LOGI(TAG, "publishing time status");
    // send time status
    esp_ble_mesh_time_status_cb_t time_status = {0};
    populate_time_status(&time_status);
    uint8_t prev_ttl = time_server.model->pub->ttl;
    time_server.model->pub->ttl = 0;
    esp_ble_mesh_model_publish(time_server.model, ESP_BLE_MESH_MODEL_OP_TIME_STATUS, sizeof(time_status), (uint8_t *) &time_status, ROLE_NODE);
    time_server.model->pub->ttl = prev_ttl;

    // update last publish time
    last_status_publish_ts = esp_timer_get_time();
  }
}

esp_err_t ble_mesh_time_init(void) {
  bool exist;
  ESP_RETURN_ON_ERROR(ble_mesh_nvs_restore(APP_BLE_MESH_NVS_HANDLE, NVS_KEY, &persist_state, sizeof(persist_state), &exist), TAG, "failed to restore time server state from nvs flash");

  esp_timer_create_args_t timer_opts = {
    .name = "esp_ble_mesh_tz_upd",
    .skip_unhandled_events = false,
    .dispatch_method = ESP_TIMER_TASK,
    .callback = tz_update_timer_cb,
  };
  ESP_RETURN_ON_ERROR(esp_timer_create(&timer_opts, &tz_update_timer), TAG, "failed to create tz update timer");
  if (exist) {
    time_state.time_role = persist_state.time_role;
    time_state.time.time_authority = persist_state.time_authority;
    time_state.time.time_zone_offset_curr = persist_state.time_zone_offset_curr;
    time_state.time.time_zone_offset_new = persist_state.time_zone_offset_new;
    time_state.time.tai_utc_delta_curr = persist_state.tai_utc_delta_curr;
    time_state.time.tai_utc_delta_new = persist_state.tai_utc_delta_new;
    time_state.time.uncertainty = persist_state.uncertainty;
    memcpy(time_state.time.tai_delta_change, persist_state.tai_delta_change, sizeof(persist_state.tai_delta_change));
    memcpy(time_state.time.tai_zone_change, persist_state.tai_zone_change, sizeof(persist_state.tai_zone_change));

    // update current timezone
    set_timezone(persist_state.time_zone_offset_curr, NULL, false);

    //reschedule tz update
    if (!(persist_state.tai_zone_change[0] == 0x00 &&
          persist_state.tai_zone_change[1] == 0x00 &&
          persist_state.tai_zone_change[2] == 0x00 &&
          persist_state.tai_zone_change[3] == 0x00 &&
          persist_state.tai_zone_change[4] == 0x00
    )) {
      set_timezone(persist_state.time_zone_offset_new, persist_state.tai_zone_change, false);
    } else {
      esp_timer_stop_blocking(tz_update_timer, pdMS_TO_TICKS(1000));
      time_state.time.time_zone_offset_new = time_state.time.time_zone_offset_curr;
      memset(time_state.time.tai_zone_change, 0x00, sizeof(time_state.time.tai_zone_change));
    }
  }
  return ESP_OK;

}

esp_err_t ble_mesh_time_unprov() {
  /*
   * TODO decide how to propagate errors here.
   * we need to do all of the things here on a best effort basis
   */
  esp_timer_stop_blocking(tz_update_timer, pdMS_TO_TICKS(1000));
  esp_timer_delete(tz_update_timer);
  ESP_RETURN_ON_ERROR(ble_mesh_nvs_erase(APP_BLE_MESH_NVS_HANDLE, NVS_KEY), TAG, "failed to erase persisted nvs time");
  return ESP_OK;
}
