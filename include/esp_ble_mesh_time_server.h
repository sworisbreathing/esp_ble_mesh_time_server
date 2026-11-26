/* esp_ble_mesh_time_server.h - BLE Mesh time server hooks */

/*
 * SPDX-FileCopyrightText: 2026 Steven Swor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef BLE_MESH_TIME_SCENE_SERVER_H_
#define BLE_MESH_TIME_SCENE_SERVER_H_

#include "esp_ble_mesh_defs.h"
#include "esp_ble_mesh_time_scene_model_api.h"
#include "nvs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Max message size for time server.
 *
 * @note Max message sizes are calculated from message descriptions in section 5.2.1
 * of the BLE Mesh spec, v1.1.1.
 */
#define MAX_TIME_SERVER_MSG_SIZE 11

/**
 * @brief Max message size for time setup server.
 *
 * @note Max message sizes are calculated from message descriptions in section 5.2.1
 * of the BLE Mesh spec, v1.1.1.
 */
#define MAX_TIME_SETUP_SERVER_MSG_SIZE 11

/**
 * @brief UTC time of the TAI epoch, in seconds.
 *
 * @note TAI epoch is `2000-01-01T00:00:00`, which corresponds to UTC time `1999-12-31T23:59:28`.
 */
#define TAI_UTC_EPOCH_SECONDS 946684768

/**
 * @brief The value of TAI-UTC delta at the TAI epoch.
 */
#define TAI_UTC_OFFSET_AT_EPOCH 32

/**
 * @brief The TAI-UTC delta as of 31 December, 2016.
 *
 * @note The BIPM voted in 2026 to replace leap seconds with leap hours.
 *       Leap hours are not expected to be added for several centuries.
 */
#define DEFAULT_TAI_UTC_OFFSET 37

/**
 * @brief BLE Mesh Time state value for a TAI-UTC delta of 0.
 *        TAI-UTC delta values below this number are negative.
 */
#define TAI_DELTA_ZERO 0x00FF

/**
 * @brief BLE Mesh Time state value for a Time Zone offset of 0.
 *        Time Zone offset values below this number are negative.
 */
#define TIME_ZONE_OFFSET_ZERO 0x40

/**
 * @brief Define a time state populated with default values.
 *
 * @param _name Variable name given to the time state.
 */
#define ESP_BLE_MESH_TIME_STATE_DEFINE(_name) \
    static esp_ble_mesh_time_state_t _name = { \
        .time.tai_utc_delta_curr = TAI_DELTA_ZERO + DEFAULT_TAI_UTC_OFFSET, \
        .time.tai_utc_delta_new = TAI_DELTA_ZERO + DEFAULT_TAI_UTC_OFFSET, \
        .time.time_zone_offset_curr = TIME_ZONE_OFFSET_ZERO, \
        .time.time_zone_offset_new = TIME_ZONE_OFFSET_ZERO, \
        .time_role = ESP_BLE_MESH_TIME_NONE, \
    }

/**
 * @brief Define a time server.
 *
 * @param _name       Variable name tiven to the time server.
 * @param _state_data Pointer to the time state.
 */
#define ESP_BLE_MESH_TIME_SRV_DEFINE(_name, _state_data) \
    static esp_ble_mesh_time_srv_t _name = { \
        .rsp_ctrl = { \
            .get_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP, \
            .set_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP, \
            .status_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP, \
        }, \
        .state = _state_data, \
    }

/**
 * @brief Define a time setup server.
 *
 * @param _name       Variable name given to the time setup server.
 * @param _state_data Pointer to the time state.
 */
#define ESP_BLE_MESH_TIME_SETUP_SRV_DEFINE(_name, _state_data) \
    static esp_ble_mesh_time_setup_srv_t _name = { \
        .rsp_ctrl = { \
            .get_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP, \
            .set_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP, \
            .status_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP, \
        }, \
        .state = _state_data, \
    }

/**
 * @brief BLE Mesh time server configuration.
 */
typedef struct {
  /**
   * @brief Pointer to the BLE Mesh time server.
   *
   * This pointer must remain valid during the lifetime
   * of the BLE Mesh device.
   * Shall not be set to NULL.
   * State property must not be NULL and must be shared
   * with time_setup_server.
   */
  esp_ble_mesh_time_srv_t *time_server;
  /**
   * @brief Pointer to the BLE Mesh time setup server.
   *
   * This pointer must remain valid during the lifetime
   * of the BLE Mesh device.
   * Shall not be set to NULL.
   * State property must not be NULL and must be shared
   * with time_server.
   */
  esp_ble_mesh_time_setup_srv_t *time_setup_server;
  /**
   * @brief NVS Handle for settings persistence.
   *
   * When time server persistence is enabled via `CONFIG_ESP_BLE_MESH_TIME_SETTINGS`,
   * this must be a properly initialized handle which must remain valid during
   * the lifetime of the BLE Mesh device.
   */
  nvs_handle_t nvs_handle;
} esp_ble_mesh_time_server_config_t;

/**
 * @brief time server callback provided by this component
 *
 * This callback should be registered via:
 *
 * ```c
 * esp_ble_mesh_register_time_scene_server_callback(ble_mesh_time_scene_server_cb);
 * ```
 *
 * @param[in] event Event type
 * @param[in] param Pointer to callback parameter
 */
void ble_mesh_time_scene_server_cb(esp_ble_mesh_time_scene_server_cb_event_t event,
                                                esp_ble_mesh_time_scene_server_cb_param_t *param);

/**
 * @brief Hook to update BLE Mesh Time state from an out-of-band time source
 * (GPS, SNTP, etc)
 *
 * @param[in] uncertainty - the time source uncertainty in 10ms steps
 */
void oob_time_updated(uint8_t uncertainty);

/**
 * @brief Initialises BLE Mesh Time functionality.
 *
 * @note This API needs to be called after BLE Mesh Time callbacks are registered
 *       and BLE Mesh is initialized.
 *
 *       The time server, time setup server, and state must be defined,
 *       and the state must be set on both the time server and time setup server.
 *
 * @param[in] config: The time server configuration. Its contents must remain valid for the
 *                    life of the BLE Mesh device, however the configuration itself need only
 *                    remain valid until this call completes.
 *
 * @return
 *    - ESP_OK: success
 *    - ESP_ERR_INVALID_ARG if time_server or time_setup_server are NULL or their state is not
 *                           properly bound
 *    - error codes returned by \c nvs_get_blob
 *    - error codes returned by \c esp_timer_create
 */
esp_err_t ble_mesh_time_init(esp_ble_mesh_time_server_config_t config);

/**
 * @brief Deinitialises BLE Mesh Time functionality.
 *
 * @note this is a best-effort cleanup routine
 *
 * @return ESP_OK on success, otherwise the relevant error code
 */
esp_err_t ble_mesh_time_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_TIME_SCENE_SERVER_H_ */
