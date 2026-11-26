# FAQs

## Time Zone

### My code already manages the system timezone

The current implementation expects timezone to be managed through BLE Mesh.
Hopefully this will be addressed soon.

## Time Uncertainty

### How should OOB time sources calculate uncertainty?

If your OOB source/library provides any documentation with regard to
clock drift/skew, start with that, and measure or estimate processing
time to come up with a reasonable value for uncertainty.

If you aren't sure, provide a conservative estimate.

The argument to `oob_time_updated()` has 10ms resolution, so a value
of `1` is an uncertainty of 10ms, `2` means 20ms. Uncertainties greater
than 2.55s should be clamped to `255`.

### Why does uncertainty keep increasing?

The current implementation adds a fixed 30ms uncertainty for each hop
through a time relay node. Future versions might make this configurable
or do a smarter measurement/estimation.

## Troubleshooting

### My devices aren't sending or receiving BLE Mesh time messages

First, rule out the simple stuff:

1. Confirm the devices are powered on
2. Confirm the devices are in range of each other and not falling prey to RF interference
3. Confirm the devices are provisioned as nodes to the same BLE Mesh subnetwork
4. Confirm the correct AppKeys are bound to the time server and time setup server

   **Note:** The BLE Mesh spec
   [strongly recommends](https://www.bluetooth.com/specifications/specs/html/?src=mmdl_v1-1-1_1768589344/MMDL_v1.1.1/out/en/index-en.html#UUID-98848456-4dc1-ca0d-20ce-f817592aed4a)
   using a dedicated AppKey for the time setup server.

5. Confirm your IDF version to rule out a
   [long-standing bug](https://github.com/espressif/esp-idf/issues/13241).

    - 5.3 - requires 5.3.7 or higher
    - 5.4 - requires 5.4.5 or higher
    - 5.5 - requires 5.5.7 or higher
    - 6.0 - requires 6.0.4 or higher
    - 6.1 - requires 6.1.2 or higher

    **Note:** at the time of this writing, none of these IDF versions have been released yet.

If the node correctly handles Time Set / Time Get messages and the issue is limited to the handling of unsolicited Time Status messages (i.e. via `oob_time_updated()`), it's likely related to the node's time role:

- `ESP_BLE_MESH_TIME_NONE` is the default. These nodes silently ignore unsolicited Time Status
  messages entirely.
- `ESP_BLE_MESH_TIME_AUTHORITY` nodes can send unsolicited Time Status messages (i.e. via `oob_time_updated()`) but silently ignore any messages received from other nodes
- `ESP_BLE_MESH_TIME_RELAY` nodes process incoming unsolicited Time Status messages and propagate them through the network. They can also send their own unsolicited Time Status messages.
- `ESP_BLE_MESH_TIME_CLIENT` nodes process incoming unsolicited Time Status messages but don't send/propagate them.

If the time role isn't the issue, then the issue is likely related to publish/subscription settings or network topology:

- Check the publish address of your Time Authority / Time Relay nodes
- Check the subscription address of your Time Relay / Time Client nodes
- Unsolicited Time Status messages are always sent with a TTL of 0. Nodes must be in direct communication with a Time Authority or Time Relay node in order to receive these messages.

### The nRF Mesh app isn't sending or receiving BLE Mesh time messages

[See above](#my-devices-arent-sending-or-receiving-ble-mesh-time-messages). The most common reason this
can happen is the byte alignment issue affecting older IDF versions.

### The time displayed in the nRF Mesh app is slightly off from the time reported by my device

There are two bugs in nRF Mesh which could be affecting you.

1. Older versions of nRF Mesh
   [used the wrong TAI epoch](https://github.com/nordicsemi/Android-nRF-Mesh-Library/issues/652).
   Updating nRF Mesh to the latest version should fix this issue.
2. nRF Mesh doesn't adjust for TAI-UTC delta when rendering human-readable time. I'm hoping to
   raise this soon, along with a PR to fix it.

### My device's system time isn't updating

If none of the [above troubleshooting steps](#my-devices-arent-sending-or-receiving-ble-mesh-time-messages) helps, check the logs for a message such as

```text
E (1234) ble_mesh_time: failed to set system time: ...
```
and note the error name at the end.

- `EPERM` means your app doesn't have permission to change the system time. On Linux targets, `CAP_SYS_TIME` is required.
- If you see a different error name, please open an issue and
  provide as much detail as you can.

### My device's time zone isn't updating

If none of the [above troubleshooting steps](#my-devices-arent-sending-or-receiving-ble-mesh-time-messages)
helps, it's possible you're hitting one of the following issues:

1. Insufficient memory to set the `TZ` environment variable. Check the logs for an error message saying
   `failed to set TZ`.
2. You've encountered a [race condition](guide.md#thread-safety)
3. Something went wrong with `tzset()`. Unfortunately, `tzset()` doesn't report errors. Check the
   logs for an info message saying `time zone updated` and note the values reported. If the numeric portions
   don't match (ignoring `+`, `-`, `<`, `>`, and `:`), then this is likely to be the culprit.

    Successful `tzset()`:

    ```text
    I (1234) ble_mesh_time: time zone updated to <+1100>-11:00 (+1100)
    ```

    `tzset()` failure:

    ```text
    I (1234) ble_mesh_time: time zone updated to <+1100>-11:00 (-0600)
    ```
