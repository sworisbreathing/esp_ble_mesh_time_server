/* main.c - Application main entry point */

/*
 * SPDX-FileCopyrightText: 2017 Intel Corporation
 * SPDX-FileContributor: 2018-2026 Espressif Systems (Shanghai) CO LTD
 * SPDX-FileContributor: 2026 Steven Swor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "esp_err.h"
#include "esp_log.h"
#include "nvs.h"

#if CONFIG_ESP_BLE_MESH_TIME_SETTINGS
#include "ble_mesh_example_nvs.h"
#endif /* CONFIG_ESP_BLE_MESH_TIME_SETTINGS */

#include "board.h"
#include "app/nvs_flash.h"
#include "app/ble_mesh.h"
#include "app/sntp.h"
#include "app/wifi.h"

#define TAG "main.c"

static nvs_handle_t s_nvs_handle;

void app_main(void)
{
    esp_log_level_set("EXAMPLE_NVS", ESP_LOG_WARN);
    esp_log_level_set("EXAMPLE_NVS: Store, data", ESP_LOG_WARN);
    esp_log_level_set("EXAMPLE_NVS: Restore, data", ESP_LOG_WARN);

    esp_log_level_set("wifi", ESP_LOG_WARN);
    esp_log_level_set("wifi_init", ESP_LOG_WARN);
    esp_log_level_set("esp_netif_handlers", ESP_LOG_WARN);

    esp_log_level_set("BLE_MESH", ESP_LOG_WARN);

    ESP_LOGI(TAG, "Initializing...");

    board_init();

    ESP_ERROR_CHECK(nvs_flash_init_or_erase());

#if CONFIG_ESP_BLE_MESH_TIME_SETTINGS
    ESP_ERROR_CHECK(ble_mesh_nvs_open(&s_nvs_handle));
#endif /* CONFIG_ESP_BLE_MESH_TIME_SETTINGS */

    /* Initialize the Bluetooth Mesh Subsystem */
    ESP_ERROR_CHECK(app_ble_mesh_init(s_nvs_handle));

    /* Initialize Wi-Fi */
    ESP_ERROR_CHECK(initialise_wifi());

    ESP_ERROR_CHECK(init_sntp());
}
