/*
 * SPDX-FileCopyrightText: 2026 Steven Swor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "esp_event.h"
#include "app/event_loop.h"

esp_err_t init_default_event_loop(void) {
  esp_err_t err;
  err = esp_event_loop_create_default();
  if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
    return err;
  }
  return ESP_OK;
}
