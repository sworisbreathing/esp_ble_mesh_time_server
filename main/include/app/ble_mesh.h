/* ble_mesh.h - BLE Mesh hooks */

/*
 * SPDX-FileCopyrightText: 2026 Steven Swor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _APP_BLE_MESH_H_
#define _APP_BLE_MESH_H_

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize BLE mesh subsystem, register callbacks, etc.
 * @see blueooth_init()
 * @see ble_mesh_get_dev_uuid()
 */
esp_err_t app_ble_mesh_init(void);

#ifdef __cplusplus
}
#endif

#endif /* _APP_BLE_MESH_H_ */
