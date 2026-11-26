#ifndef APP_BLE_MESH_DEFS_H_
#define APP_BLE_MESH_DEFS_H_

#include "esp_ble_mesh_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @def APP_BLE_MESH_MODEL_PUB_DEFINE
 *
 *  Define a model publication context as a non-static variable
 *
 *  @param _name    Variable name given to the context.
 *  @param _msg_len Length of the publication message.
 *  @param _role    Role of the device which contains the model.
 */
#define APP_BLE_MESH_MODEL_PUB_DEFINE(_name, _msg_len, _role) \
    NET_BUF_SIMPLE_DEFINE_STATIC(bt_mesh_pub_msg_##_name, _msg_len); \
    esp_ble_mesh_model_pub_t _name = { \
        .msg = &bt_mesh_pub_msg_##_name, \
        .update = (uint32_t)NULL, \
        .dev_role = _role, \
    }

#ifdef __cplusplus
}
#endif

#endif /* APP_BLE_MESH_DEFS_H_ */
