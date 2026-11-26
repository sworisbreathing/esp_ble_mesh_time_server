# SNTP OOB Example

This demo shows how to use SNTP as an out-of-band time source.

## How to Use This Example

### Hardware Required

1. a device with WiFi capability to act as the time authority, such as an ESP32-C3
2. a second device (with or without WiFi) to act as the time client, such as an ESP32-H2
3. a third device (such as an Android phone) to act as the provisioner / time client

### Configure the Example

#### Time Authority Node

1. Use `idf.py set-target` to set the appropriate chip
2. Configure the following settings via `idf.py menuconfig`
    - **Example Configuration -> WiFi SSID**
    - **Example Configuration -> WiFi Password**
    - (Optional) **Component config -> LWIP -> SNTP -> Request interval to update time (ms)** (default used by the example is 5 minutes)
3. Flash the device via `idf.py -p <port> flash`

#### Time Client Node

1. Use `idf.py set-target` to set the appropriate chip
2. If the device has onboard WiFi, clear the WiFi SSID and WiFi Password settings to ensure the device does not connect to WiFi or trigger time updates via SNTP
3. Flash the device via `idf.py -p <port> flash`

#### Provisioning/Time Client Device

1. Provision both nodes to the same BLE mesh network
2. Bind the same AppKey to both nodes' time servers
3. Bind an AppKey to both nodes' time setup servers

    **Note** for demo purposes, it's fine to use a single AppKey,
    but for a real mesh network it's strongly recommended to have
    a dedicated AppKey for the time setup servers which is not
    used for anything else.

4. Configure the wifi-enabled device's time setup server with the
   Mesh Time Authority role. You can also configure the timezone
   offset on the time setup server if you like.
5. Read the second device's time using either the time server
   or time setup server. If the device has only recently been
   powered on, it should be near the unix epoch (January 1, 1970).
6. Configure the second device's time setup server with the
   Mesh Time Client role. You can also configure the timezone
   offset on the time setup server if you like.

### Monitor the example

1. Monitor the device logs (e.g. via monitor output) on the time
   authority node for SNTP updates and sending oob time messages

    ```text
    I (9473) app/sntp.c: Notification of a time synchronization event
    I (9473) app/sntp.c: SNTP event: time synced (UTC): 2026-11-26 11:48:51.066074
    I (9473) ble_mesh_time: time updated from oob source with uncertainty 0
    I (9483) ble_mesh_time: publishing time status
    ```

2. Monitor the device logs (e.g. via monitor output) on the time
   client node for time updates from the time authority node

    ```text
    I (48066) ble_mesh_time: ESP_BLE_MESH_TIME_SCENE_SERVER_RECV_STATUS_MSG_EVT
    I (48066) ble_mesh_time: time set: 2026-11-26 22:48:51+1100
    ```

3. Using the provisioning / time client device, read the time from
   both nodes using either the time server or time setup server. You should see the current time (give or take a few seconds).
