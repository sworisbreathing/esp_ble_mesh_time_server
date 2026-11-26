# API Reference

## Header files

- [include/esp_ble_mesh_time_server.h](#file-includeesp_ble_mesh_time_serverh)

## File include/esp_ble_mesh_time_server.h





## Structures and Types

| Type | Name |
| ---: | :--- |
| struct | [**esp\_ble\_mesh\_time\_server\_config\_t**](#struct-esp_ble_mesh_time_server_config_t) <br>_BLE Mesh time server configuration._ |

## Functions

| Type | Name |
| ---: | :--- |
|  esp\_err\_t | [**ble\_mesh\_time\_deinit**](#function-ble_mesh_time_deinit) (void) <br>_Deinitialises BLE Mesh Time functionality._ |
|  esp\_err\_t | [**ble\_mesh\_time\_init**](#function-ble_mesh_time_init) ([**esp\_ble\_mesh\_time\_server\_config\_t**](#struct-esp_ble_mesh_time_server_config_t) config) <br>_Initialises BLE Mesh Time functionality._ |
|  void | [**ble\_mesh\_time\_scene\_server\_cb**](#function-ble_mesh_time_scene_server_cb) (esp\_ble\_mesh\_time\_scene\_server\_cb\_event\_t event, esp\_ble\_mesh\_time\_scene\_server\_cb\_param\_t \*param) <br>_time server callback provided by this component_ |
|  void | [**oob\_time\_updated**](#function-oob_time_updated) (uint8\_t uncertainty) <br>_Hook to update BLE Mesh Time state from an out-of-band time source (GPS, SNTP, etc)._ |

## Macros

| Type | Name |
| ---: | :--- |
| define  | [**DEFAULT\_TAI\_UTC\_OFFSET**](#define-default_tai_utc_offset)  37<br>_The TAI-UTC delta as of 31 December, 2016._ |
| define  | [**ESP\_BLE\_MESH\_TIME\_SETUP\_SRV\_DEFINE**](#define-esp_ble_mesh_time_setup_srv_define) (\_name, \_state\_data) <br>_Define a time setup server._ |
| define  | [**ESP\_BLE\_MESH\_TIME\_SRV\_DEFINE**](#define-esp_ble_mesh_time_srv_define) (\_name, \_state\_data) <br>_Define a time server._ |
| define  | [**ESP\_BLE\_MESH\_TIME\_STATE\_DEFINE**](#define-esp_ble_mesh_time_state_define) (\_name) <br>_Define a time state populated with default values._ |
| define  | [**MAX\_TIME\_SERVER\_MSG\_SIZE**](#define-max_time_server_msg_size)  11<br>_Max message size for time server._ |
| define  | [**MAX\_TIME\_SETUP\_SERVER\_MSG\_SIZE**](#define-max_time_setup_server_msg_size)  11<br>_Max message size for time setup server._ |
| define  | [**TAI\_DELTA\_ZERO**](#define-tai_delta_zero)  0x00FF<br>_BLE Mesh Time state value for a TAI-UTC delta of 0. TAI-UTC delta values below this number are negative._ |
| define  | [**TAI\_UTC\_EPOCH\_SECONDS**](#define-tai_utc_epoch_seconds)  946684768<br>_UTC time of the TAI epoch, in seconds._ |
| define  | [**TAI\_UTC\_OFFSET\_AT\_EPOCH**](#define-tai_utc_offset_at_epoch)  32<br>_The value of TAI-UTC delta at the TAI epoch._ |
| define  | [**TIME\_ZONE\_OFFSET\_ZERO**](#define-time_zone_offset_zero)  0x40<br>_BLE Mesh Time state value for a Time Zone offset of 0. Time Zone offset values below this number are negative._ |

## Structures and Types Documentation

### struct `esp_ble_mesh_time_server_config_t`

_BLE Mesh time server configuration._

Variables:

-  nvs\_handle\_t nvs_handle  <br>_NVS Handle for settings persistence._<br>When time server persistence is enabled via `CONFIG_ESP_BLE_MESH_TIME_SETTINGS`, this must be a properly initialized handle which must remain valid during the lifetime of the BLE Mesh device.

-  esp\_ble\_mesh\_time\_srv\_t \* time_server  <br>_Pointer to the BLE Mesh time server._<br>This pointer must remain valid during the lifetime of the BLE Mesh device. Shall not be set to NULL. State property must not be NULL and must be shared with time\_setup\_server.

-  esp\_ble\_mesh\_time\_setup\_srv\_t \* time_setup_server  <br>_Pointer to the BLE Mesh time setup server._<br>This pointer must remain valid during the lifetime of the BLE Mesh device. Shall not be set to NULL. State property must not be NULL and must be shared with time\_server.


## Functions Documentation

### function `ble_mesh_time_deinit`

_Deinitialises BLE Mesh Time functionality._
```c
esp_err_t ble_mesh_time_deinit (
    void
) 
```


**Note:**

this is a best-effort cleanup routine



**Returns:**

ESP\_OK on success, otherwise the relevant error code
### function `ble_mesh_time_init`

_Initialises BLE Mesh Time functionality._
```c
esp_err_t ble_mesh_time_init (
    esp_ble_mesh_time_server_config_t config
) 
```


**Note:**

This API needs to be called after BLE Mesh Time callbacks are registered and BLE Mesh is initialized.


The time server, time setup server, and state must be defined, and the state must be set on both the time server and time setup server.



**Parameters:**


* `config` The time server configuration. Its contents must remain valid for the life of the BLE Mesh device, however the configuration itself need only remain valid until this call completes.


**Returns:**



* ESP\_OK: success
* ESP\_ERR\_INVALID\_ARG if time\_server or time\_setup\_server are NULL or their state is not properly bound
* error codes returned by `nvs_get_blob`
* error codes returned by `esp_timer_create`
### function `ble_mesh_time_scene_server_cb`

_time server callback provided by this component_
```c
void ble_mesh_time_scene_server_cb (
    esp_ble_mesh_time_scene_server_cb_event_t event,
    esp_ble_mesh_time_scene_server_cb_param_t *param
) 
```


This callback should be registered via:


````cpp
esp_ble_mesh_register_time_scene_server_callback(ble_mesh_time_scene_server_cb);
````





**Parameters:**


* `event` Event type 
* `param` Pointer to callback parameter
### function `oob_time_updated`

_Hook to update BLE Mesh Time state from an out-of-band time source (GPS, SNTP, etc)._
```c
void oob_time_updated (
    uint8_t uncertainty
) 
```


**Parameters:**


* `uncertainty` - the time source uncertainty in 10ms steps

## Macros Documentation

### define `DEFAULT_TAI_UTC_OFFSET`

_The TAI-UTC delta as of 31 December, 2016._
```c
#define DEFAULT_TAI_UTC_OFFSET 37
```


**Note:**

The BIPM voted in 2026 to replace leap seconds with leap hours. Leap hours are not expected to be added for several centuries.
### define `ESP_BLE_MESH_TIME_SETUP_SRV_DEFINE`

_Define a time setup server._
```c
#define ESP_BLE_MESH_TIME_SETUP_SRV_DEFINE (
    _name,
    _state_data
) static esp_ble_mesh_time_setup_srv_t _name = { \
    .rsp_ctrl = { \
        .get_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP, \
        .set_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP, \
        .status_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP, \
    }, \
    .state = _state_data, \
}
```


**Parameters:**


* `_name` Variable name given to the time setup server. 
* `_state_data` Pointer to the time state.
### define `ESP_BLE_MESH_TIME_SRV_DEFINE`

_Define a time server._
```c
#define ESP_BLE_MESH_TIME_SRV_DEFINE (
    _name,
    _state_data
) static esp_ble_mesh_time_srv_t _name = { \
    .rsp_ctrl = { \
        .get_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP, \
        .set_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP, \
        .status_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP, \
    }, \
    .state = _state_data, \
}
```


**Parameters:**


* `_name` Variable name tiven to the time server. 
* `_state_data` Pointer to the time state.
### define `ESP_BLE_MESH_TIME_STATE_DEFINE`

_Define a time state populated with default values._
```c
#define ESP_BLE_MESH_TIME_STATE_DEFINE (
    _name
) static esp_ble_mesh_time_state_t _name = { \
    .time.tai_utc_delta_curr = TAI_DELTA_ZERO + DEFAULT_TAI_UTC_OFFSET , \
    .time.tai_utc_delta_new = TAI_DELTA_ZERO + DEFAULT_TAI_UTC_OFFSET , \
    .time.time_zone_offset_curr = TIME_ZONE_OFFSET_ZERO , \
    .time.time_zone_offset_new = TIME_ZONE_OFFSET_ZERO , \
    .time_role = ESP_BLE_MESH_TIME_NONE, \
}
```


**Parameters:**


* `_name` Variable name given to the time state.
### define `MAX_TIME_SERVER_MSG_SIZE`

_Max message size for time server._
```c
#define MAX_TIME_SERVER_MSG_SIZE 11
```


**Note:**

Max message sizes are calculated from message descriptions in section 5.2.1 of the BLE Mesh spec, v1.1.1.
### define `MAX_TIME_SETUP_SERVER_MSG_SIZE`

_Max message size for time setup server._
```c
#define MAX_TIME_SETUP_SERVER_MSG_SIZE 11
```


**Note:**

Max message sizes are calculated from message descriptions in section 5.2.1 of the BLE Mesh spec, v1.1.1.
### define `TAI_DELTA_ZERO`

_BLE Mesh Time state value for a TAI-UTC delta of 0. TAI-UTC delta values below this number are negative._
```c
#define TAI_DELTA_ZERO 0x00FF
```

### define `TAI_UTC_EPOCH_SECONDS`

_UTC time of the TAI epoch, in seconds._
```c
#define TAI_UTC_EPOCH_SECONDS 946684768
```


**Note:**

TAI epoch is `2000-01-01T00:00:00`, which corresponds to UTC time`1999-12-31T23:59:28`.
### define `TAI_UTC_OFFSET_AT_EPOCH`

_The value of TAI-UTC delta at the TAI epoch._
```c
#define TAI_UTC_OFFSET_AT_EPOCH 32
```

### define `TIME_ZONE_OFFSET_ZERO`

_BLE Mesh Time state value for a Time Zone offset of 0. Time Zone offset values below this number are negative._
```c
#define TIME_ZONE_OFFSET_ZERO 0x40
```


