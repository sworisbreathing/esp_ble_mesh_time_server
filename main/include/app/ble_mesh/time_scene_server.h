#ifndef APP_TIME_SCENE_SERVER_H_
#define APP_TIME_SCENE_SERVER_H_

#include "esp_ble_mesh_defs.h"
#include "esp_ble_mesh_time_scene_model_api.h"

#ifdef __cplusplus
extern "C" {
#endif

extern esp_ble_mesh_model_pub_t time_server_pub;
extern esp_ble_mesh_time_srv_t time_server;
extern esp_ble_mesh_model_pub_t time_setup_pub;
extern esp_ble_mesh_time_setup_srv_t time_setup_server;

void ble_mesh_time_scene_server_cb(esp_ble_mesh_time_scene_server_cb_event_t event,
                                                esp_ble_mesh_time_scene_server_cb_param_t *param);

void oob_time_updated(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_TIME_SCENE_SERVER_H_ */
