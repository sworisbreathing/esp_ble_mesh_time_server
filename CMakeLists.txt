idf_component_register(SRCS src/esp_ble_mesh_time_server.c
                       INCLUDE_DIRS include
                       REQUIRES nvs_flash
                       PRIV_REQUIRES bt esp_timer)
