#include <Arduino.h>
#include "main.h"           
#include "settings.h"


Ticker ticker;
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

bool stateOUT;
long timeEpoch;
long lastEpoch;

void tick()
{
  //toggle state
  int state = digitalRead(LED_BUILTIN);  // get the current state of GPIO1 pin
  digitalWrite(LED_BUILTIN, !state);     // set pin to the opposite state
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

// time client init
timeClient.begin();
timeClient.update();
Serial.println(timeClient.getFormattedTime());

// init dht22 sensor
dht.begin();

// ticker attach
ticker.attach(0.2, tick);

// web server config 
server.on("/", WebStatus);
server.on("/json",handleJSON);
server.on("/jsonesp",handleESP);
server.on("/jsonGet", HTTP_POST, [](){
        handleJSONget();
    });

// Start the server
server.begin();
Serial.println("Server started");
String webAddress = "http://" + WiFi.localIP().toString() + ":" + String(WEBSERVER_PORT) + "/";
Serial.println("Use this URL : " + webAddress);
display.clear();
display.setTextAlignment(TEXT_ALIGN_CENTER);
display.setFont(ArialMT_Plain_16);
display.drawString(64, 30, WiFi.localIP().toString());
display.drawString(64, 46, "Port: " + String(WEBSERVER_PORT));
display.display();

Serial.println ("DateTime: "+ String (daysOfTheWeek[timeClient.getDay()]) +" " + timeClient.getFormattedTime() );
LittleFS.begin();
readSettings(false);

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
      LampOut();
      tick();
      
    }
      delay(remainingTimeBudget);
      digitalWrite(LAMP_OUT,stateOUT);     
  }
 
   server.handleClient();
 
} // end loop()


void handleJSON(){
  
  DynamicJsonDocument doc(512);
  doc["Temperature"] = DHTtemp;
  doc["Humidity"] = DHThum;
  doc["SoilHumidity"] =SOILhum;
  doc["Lamp"]= String(stateOUT ? "TRUE": "FALSE");
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
 String lamp = doc["Lamp"];
 if (lamp.equals("TRUE")){ 
   stateOUT = true;
   Serial.println("lamp ON");
 }
 else{
  stateOUT = false;
  Serial.println("lamp OFF");
 }
 Serial.println(server.arg("plain"));
 server.send ( 200, "text/json", "{success:true}" );
}


void readSettings(bool log) {
  Serial.println("Reading config file...");
  if (LittleFS.exists(CONFIG) == false) {
    Serial.println("Settings File does not yet exists.");
    writeSettings();
    return;
  }
  File fr = LittleFS.open(CONFIG, "r");
  String line;
  while(fr.available()) {
    line = fr.readStringUntil('\n');
   if(log) Serial.println(line);
    if (line.indexOf("day1_isEnable=") >= 0) {
      days[1].isEnable = (bool)line.substring(line.lastIndexOf("day1_isEnable=") + 14).toInt();
    }
    if (line.indexOf("day2_isEnable=") >= 0) {
      days[2].isEnable =(bool)line.substring(line.lastIndexOf("day2_isEnable=") + 14).toInt();
    }
    if (line.indexOf("day3_isEnable=") >= 0) {
      days[3].isEnable =(bool)line.substring(line.lastIndexOf("day3_isEnable=") + 14).toInt();
    }
    if (line.indexOf("day4_isEnable=") >= 0) {
      days[4].isEnable =(bool)line.substring(line.lastIndexOf("day4_isEnable=") + 14).toInt();
    }
    if (line.indexOf("day5_isEnable=") >= 0) {
      days[5].isEnable =(bool)line.substring(line.lastIndexOf("day5_isEnable=") + 14).toInt();
      
    }
    if (line.indexOf("day6_isEnable=") >= 0) {
      days[6].isEnable =(bool)line.substring(line.lastIndexOf("day6_isEnable=") + 14).toInt(); 
    }
    if (line.indexOf("day0_isEnable=") >= 0) {
      days[0].isEnable =(bool)line.substring(line.lastIndexOf("day0_isEnable=") + 14).toInt();
    }
    if (line.indexOf("day1_start=") >= 0) {
      days[1].startHours =line.substring(line.lastIndexOf("day1_start=")+11);
    }
    if (line.indexOf("day2_start=") >= 0) {
      days[2].startHours =line.substring(line.lastIndexOf("day2_start=")+11);
    }
    if (line.indexOf("day3_start=") >= 0) {
      days[3].startHours =line.substring(line.lastIndexOf("day3_start=")+11);
    }
    if (line.indexOf("day4_start=") >= 0) {
      days[4].startHours =line.substring(line.lastIndexOf("day4_start=")+11);
    }
    if (line.indexOf("day5_start=") >= 0) {
      days[5].startHours =line.substring(line.lastIndexOf("day5_start=")+11);
    }
    if (line.indexOf("day6_start=") >= 0) {
      days[6].startHours =line.substring(line.lastIndexOf("day6_start=")+11);
    }
    if (line.indexOf("day0_start=") >= 0) {
      days[0].startHours =line.substring(line.lastIndexOf("day0_start=")+11);
    }

    if (line.indexOf("day1_stop=") >= 0) {
      days[1].stopHours =line.substring(line.lastIndexOf("day1_stop=")+10);
    }
    if (line.indexOf("day2_stop=") >= 0) {
      days[2].stopHours =line.substring(line.lastIndexOf("day2_stop=")+10);
    }
    if (line.indexOf("day3_stop=") >= 0) {
      days[3].stopHours =line.substring(line.lastIndexOf("day3_stop=")+10);
    }
    if (line.indexOf("day4_stop=") >= 0) {
      days[4].stopHours =line.substring(line.lastIndexOf("day4_stop=")+10);
    }
    if (line.indexOf("day5_stop=") >= 0) {
      days[5].stopHours =line.substring(line.lastIndexOf("day5_stop=")+10);
    }
    if (line.indexOf("day6_stop=") >= 0) {
      days[6].stopHours =line.substring(line.lastIndexOf("day6_stop=")+10);
    }
    if (line.indexOf("day0_stop=") >= 0) {
      days[0].stopHours =line.substring(line.lastIndexOf("day0_stop=")+10);
    }

    
  }
  fr.close();
 
}

void writeSettings() {
  // Save decoded message to SPIFFS file for playback on power up.
  File f = LittleFS.open(CONFIG, "w");
  if (!f) {
    Serial.println("File open failed!");
  } else {
    Serial.println("Saving settings now...");
    f.println("CONFIG");
    f.println("day1_isEnable=" +String(days[1].isEnable));
    f.println("day2_isEnable=" +String(days[2].isEnable));
    f.println("day3_isEnable=" +String(days[3].isEnable));
    f.println("day4_isEnable=" +String(days[4].isEnable));
    f.println("day5_isEnable=" +String(days[5].isEnable));
    f.println("day6_isEnable=" +String(days[6].isEnable));
    f.println("day0_isEnable=" +String(days[0].isEnable));
    f.println("day1_start="+days[1].startHours);
    f.println("day2_start="+days[2].startHours);
    f.println("day3_start="+days[3].startHours);
    f.println("day4_start="+days[4].startHours);
    f.println("day5_start="+days[5].startHours);
    f.println("day6_start="+days[6].startHours);
    f.println("day0_start="+days[0].startHours);
    f.println("day1_stop="+days[1].stopHours);
    f.println("day2_stop="+days[2].stopHours);
    f.println("day3_stop="+days[3].stopHours);
    f.println("day4_stop="+days[4].stopHours);
    f.println("day5_stop="+days[5].stopHours);
    f.println("day6_stop="+days[6].stopHours);
    f.println("day0_stop="+days[0].stopHours);
    Serial.println("Saved settings...");
  }
  
  f.close();
  readSettings(false);
  
}

void LampOut(){
  
  int startHours =  days[timeClient.getDay()].startHours.substring(0,2).toInt();
  int startMins =  days[timeClient.getDay()].startHours.substring(3,5).toInt();
  int stopHours =  days[timeClient.getDay()].stopHours.substring(0,2).toInt();
  int stopMins =  days[timeClient.getDay()].stopHours.substring(3,5).toInt();
 
  bool isEnable = days[timeClient.getDay()].isEnable;
  
  //stateOUT = isEnable;
  if (timeClient.getHours() >= startHours && timeClient.getMinutes() >= startMins && isEnable){
      stateOUT= true;
  }
  if (timeClient.getHours()>= stopHours && timeClient.getMinutes() >= stopMins && isEnable){
      stateOUT= false;
  }

}
