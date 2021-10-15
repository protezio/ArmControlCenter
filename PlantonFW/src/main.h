#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <WebServer.h>
#else
#error Invalid platform
#endif
#include <DNSServer.h>

#include <WiFiManager.h>

#include <ArduinoJson.h>
#include "SH1106Wire.h"
#include "SSD1306Wire.h"
#include "OLEDDisplayUi.h"


#include <NTPClient.h>

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Ticker.h>


#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include "LittleFS.h" 



void drawRssi(OLEDDisplay *display);
int8_t getWifiQuality();
void getUpdateTime();
void WebStatus();
void handleSystemReset();
void readSettings(bool log);
void writeSettings();
void handleWifiReset();
void handleJSON();
void handleJSONget();
void handleESP();
void handleJsonConf();
void LampOut();
void drawFrame1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) ;
void drawFrame2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) ;
void msOverlay(OLEDDisplay *display, OLEDDisplayUiState* state);