# Edited the file /usr/local/lib/python3.8/dist-packages/bleak/backends/service.py: comment the else in add_service(), add_characteristic(), add_descriptor()

import paho.mqtt.client as mqtt
import json

import asyncio
import platform
import threading

from bleak import BleakClient

from coapthon.client.helperclient import HelperClient

LED_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8"
BUTTON_UUID = "8801f158-f55e-4550-95f6-d260381b99e7"

THINGSBOARD_HOST = 'thingsboard.e-yantra.org'
ACCESS_TOKEN = 'AVsxEDvOIn27GTKUdIpP'      # Access Token of device in thingsboard

address = (
    "84:CC:A8:48:65:FA"  # <--- Change to your device's address here if you are using Windows or Linux
    if platform.system() != "Darwin"
    else "B9EA5233-37EF-4DD6-87A8-2A875E821C46"  # <--- Change to your device's address here if you are using macOS
)

esp_payload = ""
req_type = ""

coap_host = "13.250.13.141"
coap_port =5683
coap_client = HelperClient(server =(coap_host, coap_port))
coap_path = f"api/v1/{ACCESS_TOKEN}/telemetry"

def keypress_handler(sender, data):
    global req_type
    recvd_data = data.decode("utf-8")
    print(f'Received in BLE: {recvd_data}')
    parts = recvd_data.split(" ")
    coap_data = {}
    coap_data["id"] = parts[0]
    coap_data["timeTaken"] = parts[2]
    if req_type == "scan":
        coap_data["plot"] = parts[1]
    else:
        coap_data["type"] = parts[1]
    coap_data = json.dumps(coap_data)
    coap_client.post(coap_path, payload=coap_data)

async def notify(client):
    await client.start_notify(BUTTON_UUID, keypress_handler)
    while True:
        await asyncio.sleep(0.1)
        # await client.stop_notify(BUTTON_UUID)

async def run(client):
    global esp_payload
    while True:
        if not client.is_connected:
            print("Connection lost, trying again...")
            await client.connect()
            await client.start_notify(BUTTON_UUID, keypress_handler)
            print("Connected!")
        if len(esp_payload) > 0:
            print(f'Message: {esp_payload}')
            await client.write_gatt_char(LED_UUID, bytearray(esp_payload, "utf-8"))
            esp_payload = ""


        await asyncio.sleep(2.0)

async def main(address):
    async with BleakClient(address) as client:
        # await client.connect()
        await asyncio.gather(notify(client), run(client))
        # await run(client)

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("v1/devices/me/rpc/request/+")

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    global esp_payload, req_type
    #print(msg.topic+" "+str(msg.payload))
    data = json.loads(msg.payload)
    if data["method"] == "scan":
        req_type = "scan"
        d = "s"
        esp_payload = d + " " + " ".join([str(s) for s in data["params"].values()])
    else:
        req_type = "fetch"
        d = "f"
        v = ""
        vals = list(data["params"].values())
        if vals[0] == "minorInjury":
            v = "m"
        else:
            v = "M"
        esp_payload = d + " " + v + " " + " ".join([str(s) for s in data["params"].values()][1:])
    esp_payload += " -"

    print("Generated payload: ", esp_payload)
    # asyncio.run(run_new(esp_payload))
    # asyncio.run(main())
    # run_new(message)


def mqtt_thread():
    # Create a client instance
    mqttc = mqtt.Client()

    # Register connect callback 
    mqttc.on_connect = on_connect

    # Register publish message callback
    mqttc.on_message = on_message

    # Set access token
    mqttc.username_pw_set(ACCESS_TOKEN)  # username password setting

    # Connect to thingsboard using default MQTT port 1883. keepalive interval is 60 sec
    # Here THINGSBOARD_HOST acts as broker
    mqttc.connect(THINGSBOARD_HOST, port = 1883, keepalive = 10)
    
    mqttc.loop_forever()

def ble_thread():
    # Running main() for BLE
    asyncio.run(main(address))

t1 = threading.Thread(target=mqtt_thread)
t2 = threading.Thread(target=ble_thread)

t1.start()
t2.start()
