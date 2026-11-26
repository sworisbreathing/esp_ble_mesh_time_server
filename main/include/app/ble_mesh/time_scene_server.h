#ifndef APP_TIME_SCENE_SERVER_H_
#define APP_TIME_SCENE_SERVER_H_

#include "esp_ble_mesh_defs.h"
#include "esp_ble_mesh_time_scene_model_api.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  uint8_t  time_role;
  uint8_t  uncertainty;
  uint8_t  time_authority;
  uint8_t  time_zone_offset_curr;
  uint8_t  time_zone_offset_new;
  uint8_t  tai_zone_change[5];
  uint16_t tai_utc_delta_curr;
  uint16_t tai_utc_delta_new;
  uint8_t  tai_delta_change[5];
} app_time_persist_state_t;

extern esp_ble_mesh_model_pub_t time_server_pub;
extern esp_ble_mesh_time_srv_t time_server;
extern esp_ble_mesh_model_pub_t time_setup_pub;
extern esp_ble_mesh_time_setup_srv_t time_setup_server;

void ble_mesh_time_scene_server_cb(esp_ble_mesh_time_scene_server_cb_event_t event,
                                                esp_ble_mesh_time_scene_server_cb_param_t *param);

void oob_time_updated(void);

esp_err_t ble_mesh_time_init(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_TIME_SCENE_SERVER_H_ */
