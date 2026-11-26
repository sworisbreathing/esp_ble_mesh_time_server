/* generic_server.h - BLE Mesh generic server hooks */

/*
 * SPDX-FileCopyrightText: 2026 Steven Swor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _APP_BLE_MESH_GENERIC_SERVER_H_
#define _APP_BLE_MESH_GENERIC_SERVER_H_

#include "esp_ble_mesh_generic_model_api.h"
#include "nvs.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * msg len is the maximum message size from the BT Mesh model spec (including opcode)
 * see BT Mesh model spec v1.1.1, section 3.2.1
 */
#define MAX_ONOFF_MESSAGE_SIZE 6

extern esp_ble_mesh_gen_onoff_srv_t onoff_server_0;

void ble_mesh_generic_server_cb(esp_ble_mesh_generic_server_cb_event_t event,
                                               esp_ble_mesh_generic_server_cb_param_t *param);

esp_err_t generic_server_init(void);

#ifdef __cplusplus
}
#endif

#endif /* _APP_BLE_MESH_GENERIC_SERVER_H_ */
