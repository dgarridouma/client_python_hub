__author__ = 'dgarrido'

# --- DPS configuration (replaces the hardcoded connection string) ---
PROVISIONING_HOST = "global.azure-devices-provisioning.net"
ID_SCOPE = "<id-scope-from-dps>"          # from the portal, format 0ne00XXXXXX
REGISTRATION_ID = "mydevice-dps"          # must match the one used to derive the key
DEVICE_SYMMETRIC_KEY = "<derived-key-calculated-in-step-4>"

import datetime
import random
import time
import json
from azure.iot.device import ProvisioningDeviceClient, IoTHubDeviceClient, Message

period = 10

def message_handler(message):
    global period
    dict_command = json.loads(message.data)
    period = int(dict_command['period'])
    print(dict_command['message'])

def provision_device():
    """Contacts DPS and retrieves the IoT Hub connection string."""
    provisioning_client = ProvisioningDeviceClient.create_from_symmetric_key(
        provisioning_host=PROVISIONING_HOST,
        registration_id=REGISTRATION_ID,
        id_scope=ID_SCOPE,
        symmetric_key=DEVICE_SYMMETRIC_KEY,
    )
    result = provisioning_client.register()

    if result.status != "assigned":
        raise Exception(f"Provisioning failed: {result.status}")

    print(f"Device provisioned to: {result.registration_state.assigned_hub}")
    connection_string = (
        f"HostName={result.registration_state.assigned_hub};"
        f"DeviceId={result.registration_state.device_id};"
        f"SharedAccessKey={DEVICE_SYMMETRIC_KEY}"
    )
    return connection_string

def main():    
    # 1. Provisioning: the device obtains its connection string
    print("Contacting DPS...")
    connection_string = provision_device()

    # 2. From here on, same as before
    device_client = IoTHubDeviceClient.create_from_connection_string(connection_string)
    device_client.connect()
    device_client.on_message_received = message_handler

    while True:
        data = {
            'temperature': random.randint(25, 30),
            'humidity':    random.randint(50, 100),
            'pressure':    random.randint(900, 1100),
            'when':        datetime.datetime.now()
        }
        json_data = json.dumps(data, default=str)
        message = Message(json_data)
        message.content_encoding = "utf-8"
        message.content_type = "application/json"
        print(f"{data['temperature']} {data['humidity']} {data['pressure']}")
        device_client.send_message(message)
        time.sleep(period)

if __name__ == '__main__':
    main()