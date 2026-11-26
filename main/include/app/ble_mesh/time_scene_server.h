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

#define TAI_UTC_EPOCH_SECONDS 946684768
#define TAI_UTC_OFFSET_AT_EPOCH 32
#define DEFAULT_TAI_UTC_OFFSET 37
#define TAI_DELTA_ZERO 0x00FF
#define TIME_ZONE_OFFSET_ZERO 0x40

#define ESP_BLE_MESH_TIME_STATE_DEFINE(_name) \
    static esp_ble_mesh_time_state_t _name = { \
        .time.tai_utc_delta_curr = TAI_DELTA_ZERO + DEFAULT_TAI_UTC_OFFSET, \
        .time.tai_utc_delta_new = TAI_DELTA_ZERO + DEFAULT_TAI_UTC_OFFSET, \
        .time.time_zone_offset_curr = TIME_ZONE_OFFSET_ZERO, \
        .time.time_zone_offset_new = TIME_ZONE_OFFSET_ZERO, \
        .time_role = ESP_BLE_MESH_TIME_NONE, \
    }

#define ESP_BLE_MESH_TIME_SRV_DEFINE(_name, _state_data) \
    static esp_ble_mesh_time_srv_t _name = { \
        .rsp_ctrl = { \
            .get_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP, \
            .set_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP, \
            .status_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP, \
        }, \
        .state = _state_data, \
    }

#define ESP_BLE_MESH_TIME_SETUP_SRV_DEFINE(_name, _state_data) \
    static esp_ble_mesh_time_setup_srv_t _name = { \
        .rsp_ctrl = { \
            .get_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP, \
            .set_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP, \
            .status_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP, \
        }, \
        .state = _state_data, \
    }

void ble_mesh_time_scene_server_cb(esp_ble_mesh_time_scene_server_cb_event_t event,
                                                esp_ble_mesh_time_scene_server_cb_param_t *param);

void oob_time_updated(void);

/**
 * @brief Initialises BLE Mesh Time functionality.
 *
 * @note This API needs to be called after BLE Mesh Time callbacks are registered
 *       and BLE Mesh is initialized.
 *
 * @note The time server, time setup server, and state must be defined,
 *       and the state must be set on both the time server and time setup server.
 *
 * @param[in] time_server:       Pointer to the BLE Mesh time server. This pointer must
 *                               remain valid during the lifetime of the BLE Mesh device.
 *                               Shall not be set to NULL.
 *                               State property must not be NULL and must be shared
 *                               with time_setup_server.
 * @param[in] time_setup_server: Pointer to the BLE Mesh time setup server. This pointer must
 *                               remain valid during the lifetime of the BLE Mesh device.
 *                               Shall not be set to NULL.
 *                               State property must not be NULL and must be shared
 *                               with time_server.
 *
 * @return
 *    - ESP_OK: success
 *    - ESP_ERR_INVALID_ARG if time_server or time_setup_server are NULL or their state is not
 *                           properly bound
 *    - error codes returned by \c nvs_get_blob
 *    - error codes returned by \c esp_timer_create
 */
esp_err_t ble_mesh_time_init(esp_ble_mesh_time_srv_t *time_server,
                                          esp_ble_mesh_time_setup_srv_t *time_setup_server);

esp_err_t ble_mesh_time_unprov(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_TIME_SCENE_SERVER_H_ */
