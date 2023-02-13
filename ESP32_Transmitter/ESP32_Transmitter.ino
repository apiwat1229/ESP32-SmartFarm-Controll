// Sending/Receiving example
#include "BLEDevice.h"
#include "config.h"
#include "ble_sensor.h"
#include <NTPClient.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "RTClib.h"

#define Sender_Txd_pin 17
#define Sender_Rxd_pin 16

const char *ssid = "sjr_family2.4G";
// const char *ssid = "Kiti_2.4G";
const char *password = "0624368558";

unsigned long previousMillis = 0;
const long interval = 100;

float batteryData;

const int buttonPin = 19;
const int ledPin = 2;
int buttonState = 0;

HardwareSerial Sender(1);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600 * 7, 3600 * 7);
RTC_DS3231 rtc;

char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
int now_hour, now_minute, now_second, now_day, now_month, now_year, now_unixtime;
String t_hour, t_minute, t_second, t_datetime;

uint8_t try_connect = 0, try_count = 15, try_to_connectWiFi = 10;

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

  Serial.println(message);
  Serial.println("");
  Sender.print(message);
  Sender.print(t_datetime);
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
    char *deviceMacAddress = FLORA_DEVICES[i];
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

void get_time() {
  // timeClient.update();

  DateTime now = rtc.now();

  now_unixtime = now.unixtime();
  now_hour = now.hour();
  now_minute = now.minute();
  now_second = now.second();
  now_day = now.day();
  now_month = now.month();
  now_year = now.year();
  daysOfTheWeek[now.dayOfTheWeek()];

  if (now_hour < 10) {
    t_hour = "0" + String(now_hour);
  } else {
    t_hour = String(now_hour);
  }
  if (now_minute < 10) {
    t_minute = "0" + String(now_minute);
  } else {
    t_minute = String(now_hour);
  }
  if (now_second < 10) {
    t_second = "0" + String(now_second);
  } else {
    t_second = String(now_second);
  }

  t_datetime = "Time " + t_hour + ":" + t_minute + ":" + t_second;
  Serial.println(t_datetime);
  delay(1000);
}
void setup() {

  Serial.begin(115200);
  Sender.begin(115200, SERIAL_8N1, Sender_Txd_pin, Sender_Rxd_pin);

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  delay(1000);

  Serial.printf("WiFi Connecting! to %s \n", ssid);
  WiFi.begin(ssid, password);
  rtc.begin();

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    try_connect++;
    if (try_connect > try_count) {
      // ESP.restart();
      Serial.println("Skip WiFi Connect Try To Connect in " + String(try_to_connectWiFi) + " Minnute");
      break;
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    timeClient.update();
    DateTime now = rtc.now();
    rtc.adjust(DateTime(timeClient.getEpochTime()));
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    timeClient.update();
    DateTime now = rtc.now();
    rtc.adjust(DateTime(timeClient.getEpochTime()));
  }

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