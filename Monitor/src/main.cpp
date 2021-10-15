#include <Arduino.h> 
#include "main.h"           
#include "settings.h"
#include "web.h"

Ticker ticker;
WiFiManager wifiManager;
SSD1306Wire  display(OLED_Address, SDA_pin, SCL_pin);

OLEDDisplayUi ui ( &display );
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600*timezone, 60000);
ESP8266WebServer server(WEBSERVER_PORT);
ESP8266HTTPUpdateServer serverUpdater;
FrameCallback frames[] = { drawFrame1, drawFrame2, drawFrame3, drawFrame4 };
int frameCount = 4;

HTTPClient http;  
StaticJsonDocument<200> doc;

// Overlays are statically drawn on top of a frame eg. a clock
OverlayCallback overlays[] = { msOverlay };
int overlaysCount = 1;

struct day 
     {
        boolean isEnable;
        String startHours;
        String stopHours;
     } ;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

day days[8];

bool stateOUT,statePUMP;
long timeEpoch;
long lastEpoch;

String getHeader() {
  return getHeader(false);
}

void msOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  String time = timeClient.getFormattedTime();
  int wd = display->getStringWidth(time) + 2;
  display->drawString(display->getWidth() - wd,0,time);
 
 }


void drawFrame1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_16);
}
void drawFrame2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_16);
  
}
void drawFrame3(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
 
}
void drawFrame4(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
       
}

void tick(){
  //toggle state
  int state = digitalRead(LED_BUILTIN);  // get the current state of GPIO1 pin
  digitalWrite(LED_BUILTIN, !state);     // set pin to the opposite state
}

void setup(){

  unsigned long startedAt = millis();
  Serial.begin(115200);
  Serial.println("START");
  pinMode(LED_BUILTIN, OUTPUT);
  display.init();
  if (INVERT_DISPLAY) {
    display.flipScreenVertically(); // connections at top of OLED display
  }
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setContrast(255); // default is 255
  display.drawString(64, 5, "MONITOR\nV" + String(VERSION));
  display.display();
  delay(1000);
    
  if (WiFi.SSID()!="") wifiManager.setConfigPortalTimeout(60);

  if (!wifiManager.startConfigPortal("PLANTON_AP"))  //Delete these two parameters if you do not want a WiFi password on your configuration access point
  {
     Serial.println("Not connected to WiFi but continuing anyway.");
  } 
  else 
  {
     //if you get here you have connected to the WiFi
     Serial.println("connected...yeey :)");
     
  }
  
  Serial.print("After waiting ");
  int connRes = WiFi.waitForConnectResult();
  float waited = (millis()- startedAt);
  Serial.print(waited/1000);
  Serial.print(" secs in setup() connection result is ");
  Serial.println(connRes);
  if (WiFi.status()!=WL_CONNECTED)
  {
      Serial.println("failed to connect, finishing setup anyway");
  } 
  else
  {
    Serial.print("local ip: ");
    Serial.println(WiFi.localIP());
   
    WiFi.softAPdisconnect (true);
  }
   
  ui.setTargetFPS(30);
  ui.setFrameAnimation(SLIDE_LEFT);
  ui.setFrames(frames, frameCount);
  ui.setOverlays(overlays, overlaysCount);
  ui.init();
  if (INVERT_DISPLAY) {
    display.flipScreenVertically();  //connections at top of OLED display
  }
  ui.enableAutoTransition();
  ui.disableAllIndicators();
  //if you get here you have connected to the WiFi
  Serial.println("Connected...");
  timeClient.begin();
  timeClient.update();
  Serial.println(timeClient.getFormattedTime());
  if (WEBSERVER_ENABLED) {
    server.on("/", WebStatus);
    server.on("/systemreset", handleSystemReset);
    server.on("/forgetwifi", handleWifiReset);
    server.on("/updateconfig", handleUpdateConfig);
    server.on("/configure", handleConfigure);
    
    // Start the server
    server.begin();
    Serial.println("Server started");
    // Print the IP address
    String webAddress = "http://" + WiFi.localIP().toString() + ":" + String(WEBSERVER_PORT) + "/";
    Serial.println("Use this URL : " + webAddress);
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_10);
    display.drawString(64, 10, "Web Interface On");
    display.drawString(64, 20, "You May Connect to IP");
    display.setFont(ArialMT_Plain_16);
    display.drawString(64, 30, WiFi.localIP().toString());
    display.drawString(64, 46, "Port: " + String(WEBSERVER_PORT));
    display.display();
  } else {
    Serial.println("Web Interface is Disabled");
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_10);
    display.drawString(64, 10, "Web Interface is Off");
    display.drawString(64, 20, "Enable in Settings.h");
    display.display(); 
  } 
 
  Serial.println ("DateTime: "+ String (daysOfTheWeek[timeClient.getDay()]) +" " + timeClient.getFormattedTime() );
  SPIFFS.begin();
  readSettings(false);
}

void loop(){
  
  timeEpoch = timeClient.getEpochTime();
  int remainingTimeBudget = ui.update();
  if (remainingTimeBudget > 0) 
  {
    if (timeEpoch >= lastEpoch + refresh)
    {
      getUpdateTime();
      tick();
      
      http.begin("http://192.168.178.33/stat.php/HTTP/Basic/");
      http.setAuthorization("roberto", "rpbertp");
      int httpCode = http.GET();
      //Check the returning code                                                                  
      if (httpCode > 0) {
      // Get the request response payload
      String payload = http.getString();
      Serial.println(payload);
      http.end();   //Close connection
      deserializeJson(doc, payload);
      int total = doc["total_space1"].as<int>();
      int free = doc["free_space1"].as<int>();
      Serial.println(free+"/"+total);
     
    }
      delay(remainingTimeBudget);
            
    }
    
      
  }
  if (WEBSERVER_ENABLED) {
    server.handleClient();
  }
}

void drawRssi(OLEDDisplay *display){
  int8_t quality = getWifiQuality();
  for (int8_t i = 0; i < 4; i++) {
    for (int8_t j = 0; j < 3 * (i + 2); j++) {
      if (quality > i * 25 || j == 0) {
        display->setPixel(114 + 4 * i, 63 - j);
      }
    }
  }
}

// converts the dBm to a range between 0 and 100%
int8_t getWifiQuality(){
  int32_t dbm = WiFi.RSSI();
  if(dbm <= -100) {
      return 0;
  } else if(dbm >= -50) {
      return 100;
  } else {
      return 2 * (dbm + 100);
  }
}

void getUpdateTime(){
  //Update the Time
  timeClient.update();
  lastEpoch = timeClient.getEpochTime();
}

void WebStatus(){
  String btnDisable = "";
  if (!days[timeClient.getDay()].isEnable)btnDisable = "";
  else btnDisable = "disabled";
  String html = "";
  server.sendHeader("Cache-Control", "no-cache, no-store");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  server.sendContent(String(getHeader(true)));
  html +="<div class='w3-container'>";
  html += "<div class='w3-card-4' style='width:100%;'>";
  html += "<header class='w3-container w3-blue-grey'><i class='fa fa-home''></i> Status</header>";
  html+= "<div class='w3-container'>";
  html+="<p><i class='fa fa-thermometer-full'></i> Temperature:\t";   
  html+="°C";
  html+="<br></p>";
  html+="<p><i class='fa fa-percent'></i> Room Humidity:\t";
  html+="%";
  html+="<br></p>";
  html+="<p><i class='fa fa-percent'></i> Soil Humidity:\t";
  html+="%";
  html+="<br></p>";
  html+="<p><i class='fa fa-lightbulb-o'></i> Lamp:\t";
  html+=String(stateOUT ? "ON" : "OFF");
  html += "</p>";
  html+="<p><i class='fa fa-tint'></i> Pump:\t";
  html+=String(statePUMP ? "ON" : "OFF");
  html += "</p>";
  html += "<hr>";
  html += "<form class='w3-container' action='/toggleLamp' method='get'>";
  html += "<button class='w3-button w3-block blue-grey w3-round-small w3-padding-small w3-border 'type='submit' "+btnDisable+ ">LAMP</button></form>";
  html += "<br>";
  html += "<form class='w3-container' action='/togglePump' method='get'>";
  html += "<button class='w3-button w3-block blue-grey w3-round-small w3-padding-small w3-border 'type='submit'>PUMP </button></form>";
  html += "<br>";
  html +="</div></div></div>";
  server.sendContent(html); 
  html = ""; // fresh start
  server.sendContent(String(getFooter()));
  server.sendContent("");
  server.client().stop();
}

void handleChart(){
  server.sendHeader("Cache-Control", "no-cache, no-store");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  server.sendContent(String(getHeader(true)));
  String html = "";
  html +="<div class='w3-container'>";
  html += "<div class='w3-card-4' style='width:100%;'>";
  html += "<header class='w3-container w3-blue-grey'><i class='fa fa-home''></i> Chart</header>";


  server.sendContent(html); 
  html = ""; // fresh start
  server.sendContent(String(getFooter()));
  server.sendContent("");
  server.client().stop();
  ;
}


void handleConfigure(){
  
  String html = "";

  server.sendHeader("Cache-Control", "no-cache, no-store");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");

  html = getHeader();
  server.sendContent(html);
  
  String form = CHANGE_FORM;
  
  String isONlunedi = "";
  if (days[1].isEnable) {
    isONlunedi = "checked='checked'";
  }
  form.replace("%LUN%", isONlunedi);

  String isONmartedi = "";
  if (days[2].isEnable) {
    isONmartedi = "checked='checked'";
  }
  form.replace("%MAR%", isONmartedi);
 
  String isONmercoledi = "";
  if (days[3].isEnable) {
    isONmercoledi = "checked='checked'";
  }
  form.replace("%MER%", isONmercoledi);
 
 String isONgiovedi = "";
  if (days[4].isEnable) {
    isONgiovedi = "checked='checked'";
  }
  form.replace("%GIO%", isONgiovedi);

  String isONvenerdi = "";
  if (days[5].isEnable) {
    isONvenerdi = "checked='checked'";
  }
  form.replace("%VEN%", isONvenerdi);

  String isONsabato = "";
  if (days[6].isEnable) {
    isONsabato = "checked='checked'";
  }
  form.replace("%SAB%", isONsabato);
  
  String isONdomenica = "";
  if (days[0].isEnable) {
    isONdomenica = "checked='checked'";
  }
  form.replace("%DOM%", isONdomenica);

  form.replace("%start_lun%",days[1].startHours);
  form.replace("%start_mar%", days[2].startHours);
  form.replace("%start_mer%", days[3].startHours);
  form.replace("%start_gio%", days[4].startHours);
  form.replace("%start_ven%", days[5].startHours);
  form.replace("%start_sab%", days[6].startHours);
  form.replace("%start_dom%", days[0].startHours);

  form.replace("%stop_lun%", days[1].stopHours);
  form.replace("%stop_mar%", days[2].stopHours);
  form.replace("%stop_mer%", days[3].stopHours);
  form.replace("%stop_gio%", days[4].stopHours);
  form.replace("%stop_ven%", days[5].stopHours);
  form.replace("%stop_sab%", days[6].stopHours);
  form.replace("%stop_dom%", days[0].stopHours);

  String options = "<option>1</option><option>5</option><option>10</option><option>15</option><option>20</option>";
  options.replace(">"+String(refresh)+"<", " selected>"+String(refresh)+"<");
  form.replace("%OPTIONS%", options);

  server.sendContent(form);

  html = getFooter();
  server.sendContent(html);
  server.sendContent("");
  server.client().stop();
  
}

void handleUpdateConfig() {
 days[1].startHours = server.arg("startLun");
 days[2].startHours = server.arg("startMar");
 days[3].startHours = server.arg("startMer");
 days[4].startHours = server.arg("startGio");
 days[5].startHours = server.arg("startVen");
 days[6].startHours = server.arg("startSab");
 days[0].startHours = server.arg("startDom"); 
 
 days[1].isEnable = server.hasArg("cb_lun");
 days[2].isEnable = server.hasArg("cb_mar");
 days[3].isEnable = server.hasArg("cb_mer");
 days[4].isEnable = server.hasArg("cb_gio");
 days[5].isEnable = server.hasArg("cb_ven");
 days[6].isEnable = server.hasArg("cb_sab");
 days[0].isEnable = server.hasArg("cb_dom");

 days[1].stopHours = server.arg("stopLun");
 days[2].stopHours = server.arg("stopMar");
 days[3].stopHours = server.arg("stopMer");
 days[4].stopHours = server.arg("stopGio");
 days[5].stopHours = server.arg("stopVen");
 days[6].stopHours = server.arg("stopSab");
 days[0].stopHours = server.arg("stopDom");
 writeSettings();
 redirectHome();
}

String getHeader(boolean refrsh) 
{
  String menu = WEB_ACTIONS;
  String html = "<!DOCTYPE HTML>";
  html += "<html><head><title>PLANTON MONITOR</title><link rel='icon' href='data:;base64,='>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  if (refrsh) {
    html += "<meta http-equiv=\"refresh\" content=\"30\">";
  }
  html += "<link rel='stylesheet' href='https://www.w3schools.com/w3css/4/w3.css'>";
  html += "<link rel='stylesheet' href='https://www.w3schools.com/lib/w3-theme-" + themeColor + ".css'>";
  html += "<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css'>";
  html += "<link rel='icon' href='https://i.imgur.com/94T8Vp1.png' type='image/gif' sizes='16x16'>";
  html += "</head><body>";
  html += "<nav class='w3-sidebar w3-bar-block w3-card' style='margin-top:70px' id='mySidebar'>";
  html += "<div class='w3-container w3-theme-d2'>";
  //html += "<span onclick='closeSidebar()' class='w3-button w3-display-topright w3-large'><i class='fa fa-times'></i></span>";
  html += "<span onclick='closeSidebar()' class='w3-button w3-display-topright '><i class='fa fa-times'></i></span>";
  html += "<div class='w3-cell w3-left With w3-xlarge' style='width:60px'><i class='fa fa-leaf'></i></div>";
  html += "<div class='w3-padding'>MENU</div></div>";
  html += menu;
  html += "</nav>";
  //html += "<header class='w3-top w3-bar w3-theme'><button class='w3-bar-item w3-button w3-xxxlarge w3-hover-theme' onclick='openSidebar()'><i class='fa fa-bars'></i></button><h2 class='w3-bar-item'>PLANTON MONITOR</h2></header>";
  html += "<header class='w3-top w3-bar w3-theme'><button class='w3-bar-item w3-button w3-xxlarge w3-hover-theme' onclick='openSidebar()'><i class='fa fa-bars'></i></button><h3 class='w3-bar-item'>PLANTON MONITOR</h3></header>";
  html += "<script>";
  html += "function openSidebar(){document.getElementById('mySidebar').style.display='block'}function closeSidebar(){document.getElementById('mySidebar').style.display='none'}closeSidebar();";
  html += "</script>";
  html += "<br><div class='w3-container w3-large' style='margin-top:88px'>";
  return html;
}

String lastReportStatus = "";
String getFooter(){
  int8_t rssi = getWifiQuality();
  String html = "<br>";
  html += "</div>";
  html += "<footer class='w3-container w3-bottom w3-theme '>";//w3-margin-top
  //html += "<p style='text-align:left;'>";
  html += "<i class='fa fa-paper-plane-o'></i> Version: " + String(VERSION)+"<br>";
  html += "<i class='fa fa-rss'></i> Signal Strength: " + String(rssi) + "%";
  html += "<span style='float:right;'><i class='fa fa-clock-o'></i> "+String (daysOfTheWeek[timeClient.getDay()])+" "+ timeClient.getFormattedTime()+"</span>";
  //html += "</p>";
  html += "</footer>";
  html += "</body></html>";
  return html;
}

void handleSystemReset() {
  Serial.println("Reset System Configuration");
  if (SPIFFS.remove(CONFIG)) {
    redirectHome();
    ESP.restart();  
  }
}

void handleRead() {
  readSettings(true);
  redirectHome();

}


void handleToggle() {
  stateOUT = !stateOUT;
  redirectHome();

}

void handlePump(){
  statePUMP = !statePUMP;
  redirectHome();
}

void redirectHome() {
  // Send them back to the Root Directory
  server.sendHeader("Location", String("/"), true);
  server.sendHeader("Cache-Control", "no-cache, no-store");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(302, "text/plain", "");
  server.client().stop();
}

void readSettings(bool log) {
  Serial.println("Reading config file...");
  if (SPIFFS.exists(CONFIG) == false) {
    Serial.println("Settings File does not yet exists.");
    writeSettings();
    return;
  }
  File fr = SPIFFS.open(CONFIG, "r");
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
  File f = SPIFFS.open(CONFIG, "w");
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
  Serial.println("-----------");
  Serial.println("AUTO LAMP OUT");
  Serial.println("Day: " + String(timeClient.getDay()));
  int startHours =  days[timeClient.getDay()].startHours.substring(0,2).toInt();
  int startMins =  days[timeClient.getDay()].startHours.substring(3,5).toInt();
  int stopHours =  days[timeClient.getDay()].stopHours.substring(0,2).toInt();
  int stopMins =  days[timeClient.getDay()].stopHours.substring(3,5).toInt();
 
  bool isEnable = days[timeClient.getDay()].isEnable;
  
  Serial.println("isEnable: " + String(isEnable));
  Serial.println("startHours: " + String(startHours));  
  Serial.println("startMins: " + String(startMins));
  Serial.println("stopHours: " + String(stopHours));  
  Serial.println("stopMins: " + String(stopMins));
  
  //stateOUT = isEnable;
  if (timeClient.getHours() >= startHours && timeClient.getMinutes() >= startMins && isEnable){
      stateOUT= true;
  }
  if (timeClient.getHours()>= stopHours && timeClient.getMinutes() >= stopMins && isEnable){
       stateOUT= false;
  }

}
void handleWifiReset() {
   //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  redirectHome();
  WiFiManager wifiManager;
  wifiManager.resetSettings();
  ESP.restart();
}