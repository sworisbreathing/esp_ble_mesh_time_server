#ifndef APP_TIME_SCENE_SERVER_H_
#define APP_TIME_SCENE_SERVER_H_

#include "esp_ble_mesh_defs.h"
#include "esp_ble_mesh_time_scene_model_api.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Max message sizes are calculated from message descriptions in section 5.2.1
 * of the BLE Mesh spec, v1.1.1.
 */
#define MAX_TIME_SERVER_MSG_SIZE 11
#define MAX_TIME_SETUP_SERVER_MSG_SIZE 11

extern esp_ble_mesh_time_srv_t time_server;
extern esp_ble_mesh_time_setup_srv_t time_setup_server;

void ble_mesh_time_scene_server_cb(esp_ble_mesh_time_scene_server_cb_event_t event,
                                                esp_ble_mesh_time_scene_server_cb_param_t *param);

void oob_time_updated(void);

esp_err_t ble_mesh_time_init(void);

esp_err_t ble_mesh_time_unprov(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_TIME_SCENE_SERVER_H_ */
