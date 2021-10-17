#include <Arduino.h>
#include "main.h"           
#include "settings.h"


Ticker ticker;
Ticker tickerPump;

WiFiManager wifiManager;
SSD1306Wire  display(OLED_Address, SDA_pin, SCL_pin);
OLEDDisplayUi ui ( &display );
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600*timezone, 60000);
ESP8266WebServer server(WEBSERVER_PORT);
ESP8266HTTPUpdateServer serverUpdater;
FrameCallback frames[] = { drawFrame1, drawFrame2 };
int frameCount = 2;
OverlayCallback overlays[] = { msOverlay };
int overlaysCount = 1;

DHT dht(DHTPIN, DHTTYPE);
float DHTtemp = 0.0;
float DHThum = 0.0;
float SOILhum = 0.0;
struct day 
     {
        boolean isEnable;
        String startHours;
        String stopHours;
     } ;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
day days[8];

bool  stateOUT;
bool  statePUMP;
float percPUMP;
bool  autoPUMP;
long timeEpoch;
long lastEpoch;

String getDayName(int day){

switch (day)
{
case 0: return "Sunday";
    break;
case 1: return "Monday";
    break;
case 2: return "Tuesday";
    break;
case 3: return "Wednesday";
    break;
case 4: return "Thursday";
    break;
case 5: return "Friday";
    break;
case 6: return "Saturday";
    break;
default:
    return "";
  break;
}


}

void tick()
{
  //toggle state
  int state = digitalRead(LED_BUILTIN);  // get the current state of GPIO1 pin
  digitalWrite(LED_BUILTIN, !state);     // set pin to the opposite state
}

void tickPump(){
  if (autoPUMP)
    statePUMP= !statePUMP;
  else
    statePUMP = false;
}

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  String SSid = "";
 // String SSid = myWiFiManager->getDefaultAPName();
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_10);
  display.drawString(64, 15, "WiFi Not Configured");
  display.drawString(64, 25, "You May Connect to");
  display.drawString(64, 35,SSid);
  display.display();
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  ticker.attach(0.2, tick);
}

void setup() {
  
// init serial port  
Serial.begin(115200);
Serial.println("Start");

// pinout mode
pinMode(LED_BUILTIN, OUTPUT);
pinMode(LAMP_OUT, OUTPUT);
pinMode(PUMP_OUT, OUTPUT);
// display init
display.init();
if (INVERT_DISPLAY) {
   display.flipScreenVertically();
}
display.setFont(ArialMT_Plain_16);
display.setTextAlignment(TEXT_ALIGN_CENTER);
display.setContrast(255);
display.drawString(64, 5, "PLANTON\nV" + String(VERSION));
display.display();
delay(500);
ui.setTargetFPS(30);
ui.setFrameAnimation(SLIDE_LEFT);
ui.setFrames(frames, frameCount);
ui.setOverlays(overlays, overlaysCount);
ui.init();
ui.enableAutoTransition();
ui.disableAllIndicators();

// wifi manager
wifiManager.setAPCallback(configModeCallback);
   if (!wifiManager.autoConnect()) {
    Serial.println("failed to connect and hit timeout");
    delay(1000);
  };
Serial.println("Connected...");

if (!MDNS.begin("esp8266")) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
Serial.println("mDNS responder started");


// time client init
timeClient.begin();
timeClient.update();
Serial.println(timeClient.getFormattedTime());

// init dht22 sensor
dht.begin();

// ticker attach
ticker.attach(1, tick);

// web server config 
//server.on("/", WebStatus);
server.on("/json",handleJSON);
server.on("/jsonesp",handleESP);
server.on("/jsonconfig",handleJsonConf);
server.on("/jsonGet", HTTP_POST, [](){
        handleJSONget();
    });
server.on("/jsonconfigGet", HTTP_POST, [](){
        handleConfigJSONGet();
    });    

// Start the server
server.begin();
Serial.println("Server started");
String webAddress = "http://" + WiFi.localIP().toString() + ":" + String(WEBSERVER_PORT) + "/";
Serial.println("Use this URL : " + webAddress);
MDNS.addService("http", "tcp", 80);

display.clear();
display.setTextAlignment(TEXT_ALIGN_CENTER);
display.setFont(ArialMT_Plain_16);
display.drawString(64, 30, WiFi.localIP().toString());
display.drawString(64, 46, "Port: " + String(WEBSERVER_PORT));
display.display();

Serial.println ("DateTime: "+ String (daysOfTheWeek[timeClient.getDay()]) +" " + timeClient.getFormattedTime() );
LittleFS.begin();

updateConfig(readJsonSettings());


}// end setup()

void loop() {
 
  int SoilRawValue = 0.0;
  timeEpoch = timeClient.getEpochTime();
  int remainingTimeBudget = ui.update();
  if (remainingTimeBudget > 0){
    if (timeEpoch >= lastEpoch + refresh){
      getUpdateTime();
      DHTtemp = dht.readTemperature(false);
      DHThum = dht.readHumidity();
      SoilRawValue = analogRead(SOIL_PIN);
      SOILhum = map(SoilRawValue, SoilhumMin, SoilhumMax, 100.0, 0.0);
      Out();
      tick();
      
    }
      digitalWrite(LAMP_OUT,stateOUT);
      digitalWrite(PUMP_OUT,statePUMP); 
      delay(remainingTimeBudget);   
  }
 
   server.handleClient();
 
} // end loop()


void handleJSON(){
  
  DynamicJsonDocument doc(512);
  doc["Temperature"] = DHTtemp;
  doc["Humidity"] = DHThum;
  doc["SoilHumidity"] =SOILhum;
  doc["Lamp"]= String(stateOUT ? "TRUE": "FALSE");
  doc["Pump"]= String(statePUMP ? "TRUE": "FALSE");
  String buf;
  serializeJson(doc, buf);
  server.send(200, F("application/json"), buf);
  Serial.println("JSON Request...");

}

void handleESP(){

      DynamicJsonDocument doc(512);
      doc["ip"] = WiFi.localIP().toString();
      doc["gw"] = WiFi.gatewayIP().toString();
      doc["nm"] = WiFi.subnetMask().toString();
      doc["signalStrengh"] = WiFi.RSSI();
      doc["chipId"] = ESP.getChipId();
      doc["flashChipId"] = ESP.getFlashChipId();
      doc["flashChipSize"] = ESP.getFlashChipSize();
      doc["flashChipRealSize"] = ESP.getFlashChipRealSize();
      doc["freeHeap"] = ESP.getFreeHeap();
      String buf;
      serializeJson(doc, buf);
      server.send(200, F("application/json"), buf);
      Serial.println("JSONESP Request...");
     
}

void handleJSONget(){

 DynamicJsonDocument doc(512);
 deserializeJson(doc,server.arg("plain"));
  if (doc.containsKey("Lamp"))
      stateOUT= toBool(doc["Lamp"]);
  if (doc.containsKey("Pump"))
      statePUMP= toBool(doc["Pump"]);
  if (doc.containsKey("PercPump"))
      percPUMP= toBool(doc["PercPump"]);
  if (doc.containsKey("AutoPump"))
      autoPUMP= toBool(doc["AutoPump"]);    
 server.send ( 200, "text/json", "{success:true}" );
}

void handleConfigJSONGet(){
  writeJsonSettings(server.arg("plain"));
  server.send ( 200, "text/json", "{success:true}" );

}

void handleJsonConf(){
  Serial.print("\nJSON Config Request...");
  File fr = LittleFS.open(JCONFIG, "r");
  String buf;
  buf = fr.readString();
  fr.close();
  server.send(200, F("application/json"), buf); 
  
}

void getUpdateTime(){
  //Update the Time
  timeClient.update();
  lastEpoch = timeClient.getEpochTime();
}

void writeJsonSettings(String buf){

File f = LittleFS.open(JCONFIG, "w");
  if (!f) {
    Serial.print("\nFile open failed!");
  } else {
    Serial.print("\nSaving settings now...");
    f.println(buf);
    Serial.println("OK!");
   // Serial.println(buf);
    f.close();
    updateConfig(readJsonSettings());
  }
}

String readJsonSettings(){
Serial.print("\nReading Json config file...");
  if (LittleFS.exists(JCONFIG) == false) {
    Serial.print("\nSettings File does not yet exists.");
    return "";
  }
  File fr = LittleFS.open(JCONFIG, "r");
  String buf;
  buf = fr.readString();
  Serial.print("OK!");
  fr.close();
  Serial.println(buf);
  return buf;

}

void updateConfig(String buf){
  Serial.println("\nUpdate Config");
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, buf);
  
  if (error){
    Serial.println("\nDeserializationError");
    return;
  }
  percPUMP = doc["PercPump"];
  autoPUMP = doc["AutoPump"];
  for (int i = 0; i < 7; i++){
  days[i].isEnable = (bool)doc[String(i)][0];
  days[i].startHours =(String)doc[String(i)][1];
  days[i].stopHours = (String)doc[String(i)][2];
  Serial.print("\nDay: " + String(i));
  Serial.print("\n Doc: ");
  Serial.print((String)doc[String(i)][0] + ",");
  Serial.print((String)doc[String(i)][1] + ",");
  Serial.print( (String)doc[String(i)][2]);
  Serial.print("\n System: ");
  Serial.print((String)days[i].isEnable + ",");
  Serial.print(days[i].startHours + ",");
  Serial.print(days[i].stopHours);
  };
  Serial.println("PercPUMP:"+ (String)percPUMP);

}


void Out(){  
  //Serial.println("Lamp out");
  //Serial.print("\nLampOut: ");
 // Serial.println("\n Day: " + (String)timeClient.getDay());
  int startHours=  days[timeClient.getDay()].startHours.substring(0,2).toInt();
  int startMins =  days[timeClient.getDay()].startHours.substring(3,5).toInt();
  int stopHours =  days[timeClient.getDay()].stopHours.substring(0,2).toInt();
  int stopMins  =  days[timeClient.getDay()].stopHours.substring(3,5).toInt();
  int Hours = timeClient.getHours();
  int Minute = timeClient.getMinutes();
  bool isEnable =  (bool)days[timeClient.getDay()].isEnable;

  if ((Hours > startHours || (Hours == startHours && Minute >= startMins))&& isEnable){
      stateOUT = true;
  }
  if ((Hours>stopHours || (Hours == stopHours && Minute >= stopMins ))&& isEnable){
      stateOUT = false;
  }

  if ((percPUMP>=SOILhum))
  {
      tickerPump.attach(10, tickPump);
      tickPump();
      
  }else if ((percPUMP<=SOILhum))
  {
    tickerPump.detach();

  }
  
  
 /* Serial.println("autoPUMP: " + (String)autoPUMP);
  Serial.println("percPump: " + (String) percPUMP);
  Serial.println("soilHum: " + (String)SOILhum);*/

}

void msOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  String time = timeClient.getFormattedTime();
  display->drawString(3, 0,String (DHTtemp)+" °C");
  int wd = display->getStringWidth(time) + 2;
  display->drawString(display->getWidth() - wd,0,time);
 
 }

void drawFrame1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) 
{
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_16);
  display->drawString(3 + x, 15 + y,"Temp: " + String (DHTtemp)+" °C");
  display->drawString(3 + x, 35 + y,"Hum:  " + String (DHThum)+"  %");
}

void drawFrame2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_16);
  display->drawString(3 + x, 15 + y,"Soil Hum: " + String (SOILhum)+"%");
  display->drawString(3 + x, 35 + y,"Lamp    : " + String(stateOUT ? "ON": "OFF"));
}

bool toBool(String value){ 

  if (value.equals("TRUE")||value.equals("true"))
    return true;
  if (value.equals("FALSE")||value.equals("false"))
    return false;
  return false;   
}





