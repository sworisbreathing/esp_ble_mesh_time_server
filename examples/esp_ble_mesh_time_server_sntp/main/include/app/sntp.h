/* sntp.h - SNTP hooks */

/*
 * SPDX-FileCopyrightText: 2026 Steven Swor
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef APP_SNTP_H_
#define APP_SNTP_H_

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t init_sntp(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_SNTP_H_ */
