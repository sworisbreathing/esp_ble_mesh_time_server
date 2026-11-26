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

#include "esp_err.h"
#include "esp_log.h"

#include "board.h"
#include "app/nvs_flash.h"
#include "app/ble_mesh.h"
#include "app/wifi.h"

#define TAG "main.c"

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing...");

    board_init();

    ESP_ERROR_CHECK(nvs_flash_init_or_erase());

    /* Initialize the Bluetooth Mesh Subsystem */
    ESP_ERROR_CHECK(app_ble_mesh_init());

    /* Initialize Wi-Fi */
    ESP_ERROR_CHECK(initialise_wifi());
}
