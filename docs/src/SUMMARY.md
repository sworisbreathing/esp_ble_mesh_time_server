# Summary

ESP-BLE-MESH Time Server provides a BLE Mesh time server implementation which integrates ESP-BLE-MESH with the System Time API.

## Features

- Convenience macros for defining time state / time setup server / time server
- Sanity checks to ensure time state is properly bound to time setup server / time server
- BLE Mesh Time is integrated into system time:
  - Time Set message handlers update system time via `settimeofday()`
  - Time Status messages populate TAI time using `gettimeofday()`
  - Time Zone Set messages update system time zone via `TZ`/`tzset()`
- Optional handling of TAI UTC Delta Set messages (opt-in)
- Support for scheduled Time Zone / TAI UTC Delta updates
- Hook for notifying when the system time has been updated by an out-of-band source

## Motivations

The default (`ESP_BLE_MESH_SERVER_AUTO_RSP`) time server behaviour in ESP-BLE-MESH
simply treats time as any other fixed data. It is only updated in response to a
Time Set message, meaning subsequent Time Status messages will always return the
same time value until the next Time Set message is received.

This isn't how time works.

Furthermore, the default behaviour in ESP-BLE-MESH does not properly consider
[TAI of Zone Change](https://www.bluetooth.com/specifications/specs/html/?src=mmdl_v1-1-1_1768589344/MMDL_v1.1.1/out/en/index-en.html#UUID-7c96942e-bb6c-3179-2c02-dc78348bc31e)
or
[TAI of Delta Change](https://www.bluetooth.com/specifications/specs/html/?src=mmdl_v1-1-1_1768589344/MMDL_v1.1.1/out/en/index-en.html#UUID-ce303c6f-d8b6-7172-382c-1fde47d36407)
when managing its internal state or sending a Time Status message.

When time is updated by an out-of-band source such as SNTP or GPS,
ESP-BLE-MESH doesn't provide a simple means of notifying that time
has been updated. Instead, you're required to roll your own code to:

- convert from whatever time system your OOB time source uses to TAI time
- keep track of current and future timezone changes to ensure your TAI time
  is converted correctly
- keep track of current and future leap seconds/leap hours to ensure your
  TAI time is converted correctly
- call `esp_time_scene_client_set_state()` to update the time state when
  the OOB time source updates the time (thus having to leak BLE Mesh
  model/context data to external code)

This component was created as an attempt to solve the above problems
in as simple a way as is practical.

Long-term it is desirable to get fixes into ESP-BLE-MESH itself rather than
having to rely on a third-party component, but for now, this will have to do.

---

# Programming Guide

- [Time Server](index.md)

---

# API Reference

- [API Reference](api.md)
