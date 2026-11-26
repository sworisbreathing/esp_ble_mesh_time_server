/* nvs_flash.h - NVS Flash hooks */

/*
 * SPDX-FileCopyrightText: 2026 Steven Swor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _APP_NVS_FLASH_H_
#define _APP_NVS_FLASH_H_

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize nvs flash, erasing automatically if no free pages are available.
 *
 * @see nvs_flash_init(void)
 * @see nvs_flash_erase(void)
 */
esp_err_t nvs_flash_init_or_erase(void);

#ifdef __cplusplus
}
#endif

#endif /* _APP_NVS_FLASH_H_ */
