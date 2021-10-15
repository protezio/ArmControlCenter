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

#include "SH1106Wire.h"
#include "SSD1306Wire.h"
#include "OLEDDisplayUi.h"

#include <NTPClient.h>

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Ticker.h>
#include "images.h"

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>



void drawRssi(OLEDDisplay *display);
int8_t getWifiQuality();
void getUpdateTime();
String getFooter();
String getHeader(boolean refrsh);
void WebStatus();
void handleConfigure();
void handleUpdateConfig() ;
void handleSystemReset();
void readSettings(bool log);
void writeSettings();
void redirectHome();
void handleRead();
void handleToggle();
void handlePump();
void handleWifiReset();
void handleChart();
void LampOut();
void drawFrame1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) ;
void drawFrame2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) ;
void drawFrame3(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) ;
void drawFrame4(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) ;
void drawFrame5(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) ;
void msOverlay(OLEDDisplay *display, OLEDDisplayUiState* state);