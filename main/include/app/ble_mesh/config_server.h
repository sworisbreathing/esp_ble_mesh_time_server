/* config_server.h - BLE Mesh config server hooks */

/*
 * SPDX-FileCopyrightText: 2026 Steven Swor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _APP_BLE_MESH_CONFIG_SERVER_H_
#define _APP_BLE_MESH_CONFIG_SERVER_H_

#include "esp_ble_mesh_config_model_api.h"

#ifdef __cplusplus
extern "C" {
#endif

extern esp_ble_mesh_cfg_srv_t config_server;

void ble_mesh_config_server_cb(esp_ble_mesh_cfg_server_cb_event_t event,
                                              esp_ble_mesh_cfg_server_cb_param_t *param);

#ifdef __cplusplus
}
#endif

#endif /* _APP_BLE_MESH_CONFIG_SERVER_H_ */
