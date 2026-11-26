#ifndef APP_BLE_MESH_COEXIST_H_
#define APP_BLE_MESH_COEXIST_H_

#include "esp_coexist.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#if CONFIG_ESP_COEX_SW_COEXIST_ENABLE
#define COEX_BLE_SET(status) ESP_ERROR_CHECK_WITHOUT_ABORT(esp_coex_status_bit_set(ESP_COEX_ST_TYPE_BLE, status))
#define COEX_BLE_CLEAR(status) ESP_ERROR_CHECK_WITHOUT_ABORT(esp_coex_status_bit_clear(ESP_COEX_ST_TYPE_BLE, status))
#define COEX_BLE(x, status) do { \
        COEX_BLE_SET(status); \
        (x); \
        COEX_BLE_CLEAR(status); \
    } while(0)
#else
#define COEX_BLE_SET(status) do {} while(0)
#define COEX_BLE_CLEAR(status) do {} while(0)
#define COEX_BLE(x, status) (x)
#endif

#define COEX_BLE_CONFIG(x) COEX_BLE(x, ESP_COEX_BLE_ST_MESH_CONFIG)
#define COEX_BLE_TRAFFIC(x) COEX_BLE(x, ESP_COEX_BLE_ST_MESH_TRAFFIC)
#define COEX_BLE_STANDBY(x) COEX_BLE(x, ESP_COEX_BLE_ST_MESH_STANDBY)

#ifdef __cplusplus
}
#endif
#endif /* APP_BLE_MESH_COEXIST_H_ */
