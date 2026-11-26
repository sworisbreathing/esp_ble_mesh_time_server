#include "esp_ble_mesh_local_data_operation_api.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_check.h"
#include "esp_log.h"

#include "app/ble_mesh/defs.h"
#include "app/ble_mesh/generic_server.h"

/*
 * msg len is the maximum message size from the BT Mesh model spec (including opcode)
 * see BT Mesh model spec v1.1.1, section 3.2.1
 */
#define MAX_ONOFF_MESSAGE_SIZE 6

static const char *TAG = "app/ble_mesh/generic_server.c";

APP_BLE_MESH_MODEL_PUB_DEFINE(onoff_pub_0, MAX_ONOFF_MESSAGE_SIZE, ROLE_NODE);
esp_ble_mesh_gen_onoff_srv_t onoff_server_0 = {
    .rsp_ctrl = {
        .get_auto_rsp = ESP_BLE_MESH_SERVER_AUTO_RSP,
        .set_auto_rsp = ESP_BLE_MESH_SERVER_AUTO_RSP,
    },
};

#define LED_GPIO CONFIG_LED_GPIO

#if CONFIG_LED_TYPE_STRIP
#include "led_strip.h"

static led_strip_handle_t led_strip;

static void change_led_state(esp_ble_mesh_model_t *model,
                                     esp_ble_mesh_msg_ctx_t *ctx, uint8_t onoff)
{
    uint16_t primary_addr = esp_ble_mesh_get_primary_element_address();
    uint8_t elem_count = esp_ble_mesh_get_element_count();
    uint8_t i;

    if (ESP_BLE_MESH_ADDR_IS_UNICAST(ctx->recv_dst)) {
        for (i = 0; i < elem_count; i++) {
            if (ctx->recv_dst == (primary_addr + i)) {
              if (onoff) {
                ESP_ERROR_CHECK_WITHOUT_ABORT(led_strip_set_pixel(led_strip, 0, 16, 16, 16));
                ESP_ERROR_CHECK_WITHOUT_ABORT(led_strip_refresh(led_strip));
              } else {
                ESP_ERROR_CHECK_WITHOUT_ABORT(led_strip_clear(led_strip));
              }
            }
        }
    } else if (ESP_BLE_MESH_ADDR_IS_GROUP(ctx->recv_dst)) {
        if (esp_ble_mesh_is_model_subscribed_to_group(model, ctx->recv_dst)) {
          if (onoff) {
            ESP_ERROR_CHECK_WITHOUT_ABORT(led_strip_set_pixel(led_strip, 0, 16, 16, 16));
            ESP_ERROR_CHECK_WITHOUT_ABORT(led_strip_refresh(led_strip));
          } else {
            ESP_ERROR_CHECK_WITHOUT_ABORT(led_strip_clear(led_strip));
          }
        }
    } else if (ctx->recv_dst == 0xFFFF) {
      if (onoff) {
        ESP_ERROR_CHECK_WITHOUT_ABORT(led_strip_set_pixel(led_strip, 0, 16, 16, 16));
        ESP_ERROR_CHECK_WITHOUT_ABORT(led_strip_refresh(led_strip));
      } else {
        ESP_ERROR_CHECK_WITHOUT_ABORT(led_strip_clear(led_strip));
      }
    }
}

esp_err_t generic_server_init(void) {
  led_strip_config_t strip_config = {
    .strip_gpio_num = LED_GPIO,
    .max_leds = 1,
  };

#if CONFIG_LED_STRIP_BACKEND_RMT
  led_strip_rmt_config_t rmt_config = {
      .resolution_hz = 10 * 1000 * 1000, // 10MHz
      .flags.with_dma = false,
  };
  ESP_RETURN_ON_ERROR(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip), TAG, "failed to init rmt device");
#elif CONFIG_LED_STRIP_BACKEND_SPI
  led_strip_spi_config_t spi_config = {
    .spi_bus = SPI2_HOST,
    .flags.with_dma = true,
  };
  ESP_RETURN_ON_ERROR(led_strip_new_spi_device(&strip_config, &spi_config, &led_strip), TAG, "failed to init spi device");
#else
#error "unsupported LED strip backend"
#endif /* CONFIG_LED_STRIP_BACKEND_SPI */

  ESP_RETURN_ON_ERROR(led_strip_clear(led_strip), TAG, "failed to clear led strip");
  return ESP_OK;
}

#elif CONFIG_LED_TYPE_GPIO
#include "driver/gpio.h"

static void change_led_state(esp_ble_mesh_model_t *model,
                                     esp_ble_mesh_msg_ctx_t *ctx, uint8_t onoff)
{
    uint16_t primary_addr = esp_ble_mesh_get_primary_element_address();
    uint8_t elem_count = esp_ble_mesh_get_element_count();
    uint8_t i;

    if (ESP_BLE_MESH_ADDR_IS_UNICAST(ctx->recv_dst)) {
        for (i = 0; i < elem_count; i++) {
            if (ctx->recv_dst == (primary_addr + i)) {
              ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_set_level(LED_GPIO, !onoff));
            }
        }
    } else if (ESP_BLE_MESH_ADDR_IS_GROUP(ctx->recv_dst)) {
        if (esp_ble_mesh_is_model_subscribed_to_group(model, ctx->recv_dst)) {
          ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_set_level(LED_GPIO, !onoff));
        }
    } else if (ctx->recv_dst == 0xFFFF) {
      ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_set_level(LED_GPIO, !onoff));
    }
}

esp_err_t generic_server_init(void) {
  ESP_RETURN_ON_ERROR(gpio_reset_pin(LED_GPIO), TAG, "failed to reset gpio pin");
  ESP_RETURN_ON_ERROR(gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT), TAG, "failed to set gpio direction");
  return ESP_OK;
}
#else
static void change_led_state(esp_ble_mesh_model_t *model,
                                     esp_ble_mesh_msg_ctx_t *ctx, uint8_t onoff)
{
    uint16_t primary_addr = esp_ble_mesh_get_primary_element_address();
    uint8_t elem_count = esp_ble_mesh_get_element_count();
    uint8_t i;

    if (ESP_BLE_MESH_ADDR_IS_UNICAST(ctx->recv_dst)) {
        for (i = 0; i < elem_count; i++) {
            if (ctx->recv_dst == (primary_addr + i)) {
                ESP_LOGI(TAG, "Recv onoff message (unicast 0x%04x), element index %d, onoff %s",
                         ctx->recv_dst, i, onoff ? "ON" : "OFF");
            }
        }
    } else if (ESP_BLE_MESH_ADDR_IS_GROUP(ctx->recv_dst)) {
        if (esp_ble_mesh_is_model_subscribed_to_group(model, ctx->recv_dst)) {
            ESP_LOGI(TAG, "Recv onoff message (group 0x%04x), element index %d, onoff %s",
                     ctx->recv_dst, (int)(model->element->element_addr - primary_addr), onoff ? "ON" : "OFF");
        }
    } else if (ctx->recv_dst == 0xFFFF) {
        ESP_LOGI(TAG, "Recv onoff message (broadcast), element index %d, onoff %s",
                 (int)(model->element->element_addr - primary_addr), onoff ? "ON" : "OFF");
    }
}

esp_err_t generic_server_init(void) {
  return ESP_OK;
}
#endif

static void handle_gen_onoff_msg(esp_ble_mesh_model_t *model,
                                         esp_ble_mesh_msg_ctx_t *ctx,
                                         esp_ble_mesh_server_recv_gen_onoff_set_t *set)
{
    esp_ble_mesh_gen_onoff_srv_t *srv = (esp_ble_mesh_gen_onoff_srv_t *)model->user_data;

    switch (ctx->recv_op) {
    case ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_GET:
        esp_ble_mesh_server_model_send_msg(model, ctx,
            ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_STATUS, sizeof(srv->state.onoff), &srv->state.onoff);
        break;
    case ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET:
    case ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK:
        if (set->op_en == false) {
            srv->state.onoff = set->onoff;
        } else {
            /* TODO: Delay and state transition */
            srv->state.onoff = set->onoff;
        }
        if (ctx->recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET) {
            esp_ble_mesh_server_model_send_msg(model, ctx,
                ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_STATUS, sizeof(srv->state.onoff), &srv->state.onoff);
        }
        esp_ble_mesh_model_publish(model, ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_STATUS,
            sizeof(srv->state.onoff), &srv->state.onoff, ROLE_NODE);
        change_led_state(model, ctx, srv->state.onoff);
        break;
    default:
        break;
    }
}

void ble_mesh_generic_server_cb(esp_ble_mesh_generic_server_cb_event_t event,
                                               esp_ble_mesh_generic_server_cb_param_t *param)
{
    esp_ble_mesh_gen_onoff_srv_t *srv;
    ESP_LOGD(TAG, "event 0x%02x, opcode 0x%04" PRIx32 ", src 0x%04x, dst 0x%04x",
        event, param->ctx.recv_op, param->ctx.addr, param->ctx.recv_dst);

    switch (event) {
    case ESP_BLE_MESH_GENERIC_SERVER_STATE_CHANGE_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_GENERIC_SERVER_STATE_CHANGE_EVT");
        if (param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET ||
            param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK) {
            ESP_LOGD(TAG, "onoff 0x%02x", param->value.state_change.onoff_set.onoff);
            change_led_state(param->model, &param->ctx, param->value.state_change.onoff_set.onoff);
        }
        break;
    case ESP_BLE_MESH_GENERIC_SERVER_RECV_GET_MSG_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_GENERIC_SERVER_RECV_GET_MSG_EVT");
        if (param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_GET) {
            srv = (esp_ble_mesh_gen_onoff_srv_t *)param->model->user_data;
            ESP_LOGD(TAG, "onoff 0x%02x", srv->state.onoff);
            handle_gen_onoff_msg(param->model, &param->ctx, NULL);
        }
        break;
    case ESP_BLE_MESH_GENERIC_SERVER_RECV_SET_MSG_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_GENERIC_SERVER_RECV_SET_MSG_EVT");
        if (param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET ||
            param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK) {
            ESP_LOGD(TAG, "onoff 0x%02x, tid 0x%02x", param->value.set.onoff.onoff, param->value.set.onoff.tid);
            if (param->value.set.onoff.op_en) {
                ESP_LOGD(TAG, "trans_time 0x%02x, delay 0x%02x",
                    param->value.set.onoff.trans_time, param->value.set.onoff.delay);
            }
            handle_gen_onoff_msg(param->model, &param->ctx, &param->value.set.onoff);
        }
        break;
    default:
        ESP_LOGE(TAG, "Unknown Generic Server event 0x%02x", event);
        break;
    }
}
