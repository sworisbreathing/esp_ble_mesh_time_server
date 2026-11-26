/* prov.h - BLE Mesh provisioning hooks */

/*
 * SPDX-FileCopyrightText: 2026 Steven Swor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _APP_BLE_MESH_PROV_H_
#define _APP_BLE_MESH_PROV_H_

#include "esp_ble_mesh_defs.h"
#ifdef __cplusplus
extern "C" {
#endif

esp_ble_mesh_prov_t *ble_mesh_prov_defs(uint8_t *uuid);

void ble_mesh_provisioning_cb(esp_ble_mesh_prov_cb_event_t event,
  esp_ble_mesh_prov_cb_param_t *param);

#ifdef __cplusplus
}
#endif

#endif /* _APP_BLE_MESH_PROV_H_ */
