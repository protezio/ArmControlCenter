//******************************
// Start Settings
//******************************

#define VERSION 2.1

//DHT Conf
#define DHTPIN      D5
#define DHTTYPE     DHT22 

//Display
#define OLED_Address 0x3C //Indirizzo I2C del display OLED
#define SDA_pin     SDA
#define SCL_pin     SCL

//Time
#define timezone 2
#define refresh 5

//Tool
#define LAMP_OUT D6
#define PUMP_OUT D7
#define SOIL_PIN A0
const int SoilhumMax = 1024;
const int SoilhumMin = 500;

//WebServer
#define WEBSERVER_ENABLED true
#define WEBSERVER_PORT  80
String themeColor = "blue-grey";

//File 
#define CONFIG "/conf.txt"


//Display

#define INVERT_DISPLAY true