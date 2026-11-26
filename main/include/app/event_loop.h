/* event_loop.h - Event Loop hooks */

/*
 * SPDX-FileCopyrightText: 2026 Steven Swor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef APP_EVENT_LOOP_H_
#define APP_EVENT_LOOP_H_

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t init_default_event_loop();

#ifdef __cplusplus
}
#endif

#endif /* APP_EVENT_LOOP_H_ */
