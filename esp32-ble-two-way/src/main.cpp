#include <Arduino.h>
#include <BLEServer.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "tcpip_adapter.h"

#include <rom/ets_sys.h>

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "driver/uart.h"
#include "driver/gpio.h"

#define ECHO_TEST_TXD  (GPIO_NUM_32)            // Connected to AVR Rx-0 (green jumper)
#define ECHO_TEST_RXD  (GPIO_NUM_33)            // Connected to AVR Tx-0 (orange jumper)
#define ECHO_TEST_RTS  (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS  (UART_PIN_NO_CHANGE)

#define BUF_SIZE (128)

#define LED 2
#define BUTTON 0
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define LED_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define BUTTON_CHARACTERISTIC_UUID "8801f158-f55e-4550-95f6-d260381b99e7"

BLECharacteristic *ledCharacteristic;
BLECharacteristic *buttonCharacteristic;

bool deviceConnected = false;
volatile int buttonState = HIGH;

char ack_buffer[10];

void pin_ISR();

class ServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      Serial.println("Central connected XD");
      deviceConnected = true;
    };
 
    void onDisconnect(BLEServer* pServer) {
      Serial.println("Central dis-connected :(");
      deviceConnected = false;
    }
};


class ControlSwitch: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      // Serial.printf("Received '%s'\n", value.c_str());
      if (value.length() > 1) {
        Serial.printf("Received '%s'\n", value.c_str());
        // uart functionality
        sprintf(ack_buffer, &value[0]);
        uart_write_bytes(UART_NUM_1, (const char *) ack_buffer, strlen(ack_buffer));
        return;
      }
      long switchState = std::strtol(value.c_str(), NULL, 10);
      if (switchState == 0l) {
        digitalWrite(LED, LOW);
      }
      else {
        digitalWrite(LED, HIGH);
      }
    }
};


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("CUSTOM BUILD!");

  // set up pin
  pinMode(LED, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON), pin_ISR, CHANGE);

  // set up ble
  BLEDevice::init("esp32-two-way");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  BLEService *lightSwitchService = pServer->createService(SERVICE_UUID);
  
  ledCharacteristic = lightSwitchService->createCharacteristic(
                            LED_CHARACTERISTIC_UUID,
                            BLECharacteristic::PROPERTY_READ |
                            BLECharacteristic::PROPERTY_WRITE
                          );

  BLEDescriptor* descriptor = new BLEDescriptor(BLEUUID((uint16_t) 0x2901));
  descriptor->setValue("Set LED");
  ledCharacteristic->addDescriptor(descriptor);
  ledCharacteristic->addDescriptor(new BLE2902());
  ledCharacteristic->setCallbacks(new ControlSwitch());

  buttonCharacteristic = lightSwitchService->createCharacteristic(
                          BUTTON_CHARACTERISTIC_UUID,
                          BLECharacteristic::PROPERTY_NOTIFY
                        );
  // client charactersitic descriptor: required for notify
  buttonCharacteristic->addDescriptor(new BLE2902());
  lightSwitchService->start();

  // off initially
  ledCharacteristic->setValue("0");
  buttonCharacteristic->setValue("0");

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  // pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS);
    uart_driver_install(UART_NUM_1, BUF_SIZE * 2, 0, 0, NULL, 0);

    Serial.println("Setup complete!");
}

uint8_t *data_uart = (uint8_t *) malloc(BUF_SIZE);
int len_uart;
// int count = 0;

void loop() {
    len_uart = uart_read_bytes(UART_NUM_1, data_uart, BUF_SIZE, 20 / portTICK_RATE_MS);
    if(len_uart > 0) { // && count < 20) {
      // count++;
      Serial.println((char *)data_uart);
      
    }

    // if(count == 20) {
    //   Serial.println("Final: ");
    //   Serial.println(uart_data);
    //   count = 0;
    // }
}


void pin_ISR() {
  buttonState = digitalRead(BUTTON);
}
