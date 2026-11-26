#include "esp_check.h"
#include "esp_err.h"
#include "FreeRTOSConfig.h"

#include "esp_ble_mesh_defs.h"
#include "esp_ble_mesh_common_api.h"
#include "esp_ble_mesh_provisioning_api.h"
#include "esp_ble_mesh_config_model_api.h"
#include "esp_ble_mesh_generic_model_api.h"

#include "ble_mesh_example_init.h"

#include "app/ble_mesh.h"
#include "app/ble_mesh/config_server.h"
#include "app/ble_mesh/generic_server.h"
#include "app/ble_mesh/prov.h"

#define CID_ESP 0x02E5

static const char *TAG = "app/ble_mesh.c";

static uint8_t dev_uuid[16] = { 0xdd, 0xdd };

static esp_ble_mesh_model_t root_models[] = {
    ESP_BLE_MESH_MODEL_CFG_SRV(&config_server),
    ESP_BLE_MESH_MODEL_GEN_ONOFF_SRV(&onoff_pub_0, &onoff_server_0),
};

static esp_ble_mesh_elem_t elements[] = {
    ESP_BLE_MESH_ELEMENT(0, root_models, ESP_BLE_MESH_MODEL_NONE),
};

static esp_ble_mesh_comp_t composition = {
    .cid = CID_ESP,
    .element_count = ARRAY_SIZE(elements),
    .elements = elements,
};

static esp_err_t ble_mesh_init(void)
{
    esp_err_t err = ESP_OK;

    esp_ble_mesh_register_prov_callback(ble_mesh_provisioning_cb);
    esp_ble_mesh_register_config_server_callback(ble_mesh_config_server_cb);
    esp_ble_mesh_register_generic_server_callback(ble_mesh_generic_server_cb);

    provision.uuid = dev_uuid;
    err = esp_ble_mesh_init(&provision, &composition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize mesh stack (err %d)", err);
        return err;
    }

#ifdef CONFIG_IDF_TARGET
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_ble_mesh_set_unprovisioned_device_name(CONFIG_IDF_TARGET));
#endif

    ESP_LOGI(TAG, "enabling provisioning bearers");
    err = esp_ble_mesh_node_prov_enable((esp_ble_mesh_prov_bearer_t)(ESP_BLE_MESH_PROV_ADV | ESP_BLE_MESH_PROV_GATT));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable mesh node (err %d)", err);
        return err;
    }

    ESP_LOGI(TAG, "BLE Mesh Node initialized");

    return err;
}

esp_err_t app_ble_mesh_init(void) {
  ESP_RETURN_ON_ERROR(bluetooth_init(), TAG, "failed to initialise bluetooth");
  ble_mesh_get_dev_uuid(dev_uuid);
  ESP_RETURN_ON_ERROR(ble_mesh_init(), TAG, "failed to initialise ble mesh");
  return ESP_OK;
}
