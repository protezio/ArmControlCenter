
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <ESP8266HTTPUpdateServer.h>
#include "TimeClient.h"


//declairing prototypes
void configModeCallback (WiFiManager *myWiFiManager);
int8_t getWifiQuality();
void handleSystemReset();
void handleWifiReset();
void handleUpdateConfig();
void displayHome();
void flashLED(int number, int delayTime);
void redirectHome();
void handleConfigure();
String getHeader(boolean refresh);