/* nvs_flash.c - NVS Flash hooks */

/*
 * SPDX-FileCopyrightText: 2026 Steven Swor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "esp_check.h"
#include "nvs_flash.h"
#include "app/nvs_flash.h"

static const char *TAG = "app/nvs_flash.c";

esp_err_t nvs_flash_init_or_erase(void) {
  esp_err_t err;

  err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
      ESP_RETURN_ON_ERROR(nvs_flash_erase(), TAG, "failed to erase nvs flash");
      err = nvs_flash_init();
  }
  ESP_RETURN_ON_ERROR(err, TAG, "failed to init nvs flash");
  return err;
}
