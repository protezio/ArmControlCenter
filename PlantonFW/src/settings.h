//******************************
// Start Settings
//******************************

#define VERSION 2.5

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
const int SoilhumMax = 690;
const int SoilhumMin = 280;

//WebServer
#define WEBSERVER_ENABLED truea
#define WEBSERVER_PORT  80

//File 
#define CONFIG "/conf.txt"
#define JCONFIG "/json.txt"

//Display
#define INVERT_DISPLAY false