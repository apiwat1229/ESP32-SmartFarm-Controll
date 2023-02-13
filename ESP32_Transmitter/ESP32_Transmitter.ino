// Sending/Receiving example
#include "BLEDevice.h"
#include "config.h"
#include "ble_sensor.h"

#define Sender_Txd_pin 17
#define Sender_Rxd_pin 16

unsigned long previousMillis = 0;
const long interval = 100;

float batteryData;
// int moistureData, fertilityData, lightData;

const int buttonPin = 19;
const int ledPin = 2;
int buttonState = 0;

HardwareSerial Sender(1);

void sendData() {
  String message = "";
  message += ";";
  message += temperature;
  message += ";";
  message += moisture;
  message += ";";
  message += conductivity;
  message += ";";
  message += light;
  message += ";";
  message += batteryData;

  // Serial.print("Temperature : ");
  // Serial.println(temperature);

  // Serial.print("battery : ");
  // Serial.println(batteryData);

  // Serial.print("Moisture : ");
  // Serial.println(moisture);

  // Serial.print("fertility : ");
  // Serial.println(conductivity);

  // Serial.print("light : ");
  // Serial.println(light);

  Serial.println(message);
  Serial.println("");
  Sender.print(message);
  Serial.println("Update Sensor Data To Display Complete");
}

void get_BLEsensor() {
  // create a hibernate task in case something gets stuck

  Serial.println("Initialize BLE client...");
  BLEDevice::init("");
  BLEDevice::setPower(ESP_PWR_LVL_P7);

  // check if battery status should be read - based on boot count
  bool readBattery = ((bootCount % BATTERY_INTERVAL) == 0);

  // process devices
  for (int i = 0; i < deviceCount; i++) {
    int tryCount = 0;
    char* deviceMacAddress = FLORA_DEVICES[i];
    BLEAddress floraAddress(deviceMacAddress);

    while (tryCount < RETRY) {
      tryCount++;
      if (processFloraDevice(floraAddress, deviceMacAddress, readBattery, tryCount)) {
        break;
      }
      delay(1000);
    }
    delay(1500);
  }
}

void setup() {
  //Serial.begin(Baud Rate, Data Protocol, Txd pin, Rxd pin);
  Serial.begin(115200);                                              // Define and start serial monitor
  Sender.begin(115200, SERIAL_8N1, Sender_Txd_pin, Sender_Rxd_pin);  // Define and start Sender serial port

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  delay(1000);
  // increase boot count
  bootCount++;
  get_BLEsensor();
  sendData();
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    buttonState = digitalRead(buttonPin);

    if (buttonState == LOW) {

      digitalWrite(ledPin, HIGH);
      get_BLEsensor();
      sendData();
    } else {
      // turn LED off:
      digitalWrite(ledPin, LOW);
    }
  }
}