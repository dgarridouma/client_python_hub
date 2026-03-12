# client_azure_hub

Examples of IoT devices sending telemetry to **Azure IoT Hub** and receiving cloud-to-device (C2D) commands. Includes Python and ESP32 (Arduino) implementations.

All examples simulate temperature, humidity and pressure sensors and share the same structure:
- Send telemetry every few seconds
- Listen for and execute commands from the cloud

---

## Repository structure

```
/python/
    console_iothub.py       # Random values, runs on any OS
    sensehat_iothub.py      # Uses SenseHat Emulator (Raspberry Pi / desktop)
    requirements.txt
/esp32_iothub/
    esp32_iothub.ino        # Arduino sketch for ESP32
    SETUP.md                # Setup instructions (certificates, SAS token, etc.)
README.md
```

---

## Python

Tested on Windows 11, Debian 11 and Raspberry Pi 3 (Bullseye).

### Requirements

```bash
pip install -r python/requirements.txt
```

### Configuration

Edit the connection string at the top of each script:

```python
CONNECTION_STRING = "HostName=YOURHUB.azure-devices.net;DeviceId=YOURDEVICE;SharedAccessKey=..."
```

### Run

```bash
# Console version (random values)
python python/console_iothub.py

# SenseHat Emulator version
python python/sensehat_iothub.py
```

---

## ESP32 (Arduino)

Connects to Azure IoT Hub via **MQTT over TLS** (port 8883) using the `PubSubClient` library. No Azure SDK required.

See [`esp32_iothub/SETUP.md`](esp32_iothub/SETUP.md) for full setup instructions including certificate download, SAS token generation and sending C2D commands.

### Required libraries (Arduino IDE Library Manager)

- `PubSubClient` by Nick O'Leary
- `ArduinoJson` by Benoit Blanchon

### Configuration

Edit the constants at the top of `esp32_iothub.ino`:

```cpp
const char* WIFI_SSID     = "YOUR_WIFI";
const char* WIFI_PASSWORD = "YOUR_PASSWORD";
const char* IOT_HUB_HOST  = "YOURHUB.azure-devices.net";
const char* DEVICE_ID     = "YOURDEVICE";
const char* SAS_TOKEN     = "SharedAccessSignature sr=...";
const char* MQTT_USERNAME = "YOURHUB.azure-devices.net/YOURDEVICE/?api-version=2021-04-12";
```

---

## Sending C2D commands

From Azure CLI:

```bash
az iot device c2d-message send \
  --hub-name YOURHUB \
  --device-id YOURDEVICE \
  --data '{"comando": "led_on"}'
```

From Azure Portal: IoT Hub → Devices → your device → **Message to Device**.

---

## Monitoring telemetry

```bash
az iot hub monitor-events --hub-name YOURHUB --device-id YOURDEVICE
```
