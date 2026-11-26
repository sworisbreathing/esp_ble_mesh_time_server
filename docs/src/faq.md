# FAQs

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

### The nRF Mesh app isn't sending or receiving BLE Mesh time messages

[See above](#my-devices-arent-sending-or-receiving-ble-mesh-time-messages)

### The time displayed in the nRF Mesh app is slightly off from the time reported by my device

There are two bugs in nRF Mesh which could be affecting you.

1. Older versions of nRF Mesh
   [used the wrong TAI epoch](https://github.com/nordicsemi/Android-nRF-Mesh-Library/issues/652).
   Updating nRF Mesh to the latest version should fix this issue.
2. nRF Mesh doesn't adjust for TAI-UTC delta when rendering human-readable time. I'm hoping to
   raise this soon, along with a PR to fix it.
