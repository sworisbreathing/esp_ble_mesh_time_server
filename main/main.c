/* main.c - Application main entry point */

/*
 * SPDX-FileCopyrightText: 2017 Intel Corporation
 * SPDX-FileContributor: 2018-2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "esp_log.h"

#include "board.h"
#include "app/nvs_flash.h"
#include "app/ble_mesh.h"

#define TAG "main.c"

void app_main(void)
{
    esp_err_t err;

    ESP_LOGI(TAG, "Initializing...");

    board_init();

    err = nvs_flash_init_or_erase();
    if (err) {
      ESP_LOGE(TAG, "nvs_flash_init_or_erase failed (err %d)", err);
      return;
    }

    /* Initialize the Bluetooth Mesh Subsystem */
    err = app_ble_mesh_init();
    if (err) {
        ESP_LOGE(TAG, "Bluetooth mesh init failed (err %d)", err);
    }
}
