# ESP BLE Mesh Time Server User Guide

## Define Servers and States

```c

/// Time servers with shared state
ESP_BLE_MESH_TIME_STATE_DEFINE(time_state);
ESP_BLE_MESH_TIME_SRV_DEFINE(time_server, &time_state);
ESP_BLE_MESH_TIME_SETUP_SRV_DEFINE(time_setup_server, &time_state);

/// Declare root models
static esp_ble_mesh_model_t root_models[] = {
    /// ... other root models (config server, etc.)
    ESP_BLE_MESH_MODEL_TIME_SRV(&time_server_pub, &time_server),
    ESP_BLE_MESH_MODEL_TIME_SETUP_SRV(&time_setup_server),
};
```

---

## Initialise BLE Mesh Time Server

```c

static nvs_handle_t nvs_handle;

void app_main(void)
{
  // ... (init nvs flash and open handle)
  // ... (init bluetooth subsystem and get device uuid)

  ESP_ERROR_CHECK(esp_ble_mesh_register_time_scene_server_callback(ble_mesh_time_scene_server_cb));
  // ... (register additional ble mesh callbacks)

  esp_ble_mesh_time_server_config_t ts_config = {
    .time_server = &time_server,
    .time_setup_server = &time_setup_server,
    .nvs_handle = nvs_handle,
  };
  ESP_ERROR_CHECK(ble_mesh_time_init(ts_config));

  // ... (enable BLE Mesh provisioning bearers)
}

```

---

## Update Time From OOB Source

Nodes configured with Mesh Time Authority or Mesh Time Relay role publish unsolicited time status
messages when their clocks are updated by an out-of-band (OOB) time source, such as SNTP or GPS.

Use `oob_time_updated()` after updating the system time (e.g. via `settimeofday()` or
in an SNTP event handler) to publish these unsolicited time status messages.

```c
static void sntp_event_handler(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    (void)arg; (void)base; (void)id;
    const esp_netif_sntp_time_sync_t *evt = (const esp_netif_sntp_time_sync_t *)data;
    if (evt) {
        uint8_t uncertainty = // ... determine uncertainty in 10ms increments (1=10ms, 2=20ms, etc)
        oob_time_updated(uncertainty);
    }
}
```

## Leap Seconds / Leap Hours

The General Conference on Weights and Measures voted in October 2026 to
[replace leap seconds with leap hours](https://www.bipm.org/en/cgpm-2026/resolution-5).
Although this doesn't officially take effect until May 2027, no further leap seconds
are expected to be added to UTC in the interim.

Since a full leap hour is not expected to be added to UTC for several centuries,
proper support for TAI-UTC Delta functionality is provided on an opt-in basis.
The setting `CONFIG_ESP_BLE_MESH_TIME_UTC_DELTA_ENABLE` can be found in the
menuconfig at
`Component Config -> ESP BLE Mesh Time Server -> Handle TAI-UTC Delta Set Messages`.

When `CONFIG_ESP_BLE_MESH_TIME_UTC_DELTA_ENABLE` is off (which is the default),
the TAI-UTC delta remains at a constant 37s. The time setup server will ignore
the payload of any TAI-UTC Delta Set message and instead reply with the current
fixed values.

Proper handling of TAI-UTC Delta Set messages can be enabled by configuring
`CONFIG_ESP_BLE_MESH_TIME_UTC_DELTA_ENABLE`, at the expense of:

- an additional `esp_timer` for handling scheduled TAI-UTC Delta updates
- additional processing for Time Set messages / `oob_time_updated()`
- additional processing when initialising/deinitialising the time server

## Thread Safety

This library should be as thread-safe as the underlying ESP-BLE-MESH component,
with the following exceptions:

### OOB Time Updates

The current implementation of `oob_time_updated()` is normally called
from a different FreeRTOS task (such as the default event loop) than the task which
runs ESP-BLE-MESH code/callbacks (one of potentially several bluetooth profile tasks).

The following properties are modified by `oob_time_updated()`
and thus could potentially fall prey to race conditions between the task
which invokes `oob_time_updated()` and the tasks upon which ESP-BLE-MESH runs:

Properties which are written by `oob_time_updated()`:

- time state -> uncertainty
- time state -> time authority (not to be confused with the time role named `Time Authority`)
- time server -> publish TTL (temporarily set to 0 before being restored to its previous value)

Properties which are read by `oob_time_updated()`:

- time state -> uncertainty
- time state -> time authority (not to be confused with the time role named `Time Authority`)
- time state -> time role
- time server -> publish address
- time server -> publish TTL (temporarily set to 0 before being restored to its previous value)
- time state -> current TAI-UTC Delta
- time state -> current timezone offset

To mitigate the risk, it is recommended to:

- refrain from sending Time Set messages to nodes with OOB time sources
- configure nodes with OOB time sources with the Mesh Time Authority role instead of Mesh Time Relay or Mesh Time Client
- read the publish TTL on nodes configured as Mesh Time Authority / Mesh Time Relay after changing it, to
  confirm it has been properly set

### Scheduled Timezone / TAI Delta Changes

Scheduled updates to time zone offset or TAI-UTC delta (when processing a Set message with a
TAI of change in the future) are implemented using `esp_timer`, which runs from a different
FreeRTOS task than the bluetooth stack.

Properties which are read during scheduled timezone updates:

- time state -> current TAI-UTC delta
- when persistence is enabled:
    - time state -> current TZ offset

Properties which are written during scheduled timezone updates:

- time state -> new TZ offset
- time state -> TAI of timezone change
- time state -> current TZ offset
- when persistence is enabled:
    - persistent state -> new TZ offset
    - persistent state -> current TZ offset
    - persistent state -> TAI of timezone change

Properties which are read during scheduled TAI-UTC Delta updates:

- time state -> current TAI-UTC delta

Properties which are written during scheduled TAI-UTC Delta updates:

- time state -> new TAI-UTC delta
- time state -> TAI of TAI-UTC delta change
- time state -> current TAI-UTC delta
- when persistence is enabled:
    - persistent state -> new TAI-UTC delta
    - persistent state -> current TAI-UTC delta
    - persistent state -> TAI of TAI-UTC delta change

To mitigate the risk, it is recommended to:

- avoid updating the device timezone for daylight savings
- prefer immediate timezone / TAI-UTC delta changes when sending the relevant Set messages
- include the time zone when converting time to a string (e.g. include `%z` when calling `strftime()`)

---

## Settings Persistence

Settings persistence is enabled automatically when configuring BLE Mesh settings persistence
(`CONFIG_BLE_MESH_SETTINGS` in sdkconfig or **Component config -> ESP BLE Mesh Support -> Store
BLE Mesh configuration persistently** via menuconfig). If you wish to disable time server
settings persistence while keeping the out-of-the-box ESP-BLE-MESH settings persistence,
you can set `CONFIG_ESP_BLE_MESH_TIME_SETTINGS=n` in sdkconfig, or via menuconfig at **Component
config -> ESP BLE Mesh Time Server -> Persist time server settings**.

When time server persistence is enabled:

- The `esp_ble_mesh_time_server_config_t` passed to `ble_mesh_time_init()` must contain a valid
  NVS handle.
- The NVS handle must remain valid until `ble_mesh_time_deinit()` is called
- The following settings will be persisted as a blob to NVS Flash under the `ble_mesh_time` key (19 bytes plus blob overheads):
    - time role state
    - time state uncertainty
    - time state authority bit
    - current timezone offset
    - future timezone offset (including TAI of change)
    - current TAI-UTC delta
    - future TAI-UTC delta (including TAI of change)
- `ble_mesh_time_init()` will:
    - restore the above persisted state (note: TAI-UTC delta is only restored when TAI-UTC delta functionality is enabled)
    - update the system timezone
    - execute any scheduled timezone / TAI-UTC delta updates for which the scheduled time has
      already passed (i.e. a schedule that was missed due to deep sleep)
    - restart any timers for future timezone / TAI-UTC delta updates for which the scheduled
      time is still in the future
