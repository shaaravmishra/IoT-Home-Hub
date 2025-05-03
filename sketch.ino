#include <Arduino.h>
#include <ESP8266WiFi.h>  
#include "SinricPro.h"
#include "SinricProSwitch.h"
#include <map>
#include <Wire.h>
#include <Adafruit_SSD1306.h>  // OLED display library

#define WIFI_SSID     "xxxx"
#define WIFI_PASS     "xxxx"
#define APP_KEY       "xxxx"
#define APP_SECRET    "xxxx"

#define device_ID_1   "xxxx"
#define device_ID_2   "xxxx"
#define RelayPin1 14     // D5
#define RelayPin2 12     // D6
#define RelayPin3 13     // D7
#define wifiLed   2      // D4
#define BAUD_RATE 9600

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); // OLED initialization

typedef struct {
  int relayPIN;
} deviceConfig_t;

std::map<String, deviceConfig_t> devices = {
    {device_ID_1, {RelayPin1}},
    {device_ID_2, {RelayPin2}},
};

bool onPowerState(String deviceId, bool &state) {
  int relayPIN = devices[deviceId].relayPIN;
  digitalWrite(relayPIN, state);
  return true;
}

void setupRelays() {
  for (auto &device : devices) {
    int relayPIN = device.second.relayPIN;
    pinMode(relayPIN, OUTPUT);
    digitalWrite(relayPIN, HIGH);
  }
}

void setupWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int wifiRetries = 0;
  
  // Try connecting to Wi-Fi, retrying for a maximum of 30 seconds
  while (WiFi.status() != WL_CONNECTED && wifiRetries < 120) {
    delay(250);
    wifiRetries++;
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.print("Wi-Fi: Offline");
    display.display();
  }

  if (WiFi.status() == WL_CONNECTED) {
    // If Wi-Fi is connected
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.print("Wi-Fi: Connected");
    display.display();
    digitalWrite(wifiLed, HIGH);  // Optional: LED to indicate Wi-Fi is connected
  }
}

void setupSinricPro() {
  for (auto &device : devices) {
    const char *deviceId = device.first.c_str();
    SinricProSwitch &mySwitch = SinricPro[deviceId];
    mySwitch.onPowerState(onPowerState);
  }
  SinricPro.begin(APP_KEY, APP_SECRET);
  SinricPro.restoreDeviceStates(true);
}

void setup() {
  Serial.begin(BAUD_RATE);
  pinMode(wifiLed, OUTPUT);
  digitalWrite(wifiLed, LOW);
  
  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED allocation failed"));
    for (;;);
  }
  
  // Initial display message
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.print("S: Checking...");
  display.display();

  setupRelays();
  setupWiFi();
  setupSinricPro();
}

void loop() {
  SinricPro.handle();
}

