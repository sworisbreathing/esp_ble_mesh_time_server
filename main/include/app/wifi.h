/* wifi.h - wifi hooks */

/*
 * SPDX-FileCopyrightText: 2026 Steven Swor
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef APP_WIFI_H_
#define APP_WIFI_H_

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t initialise_wifi(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_WIFI_H_ */
