//******************************
// Start Settings
//******************************

#define VERSION 1.0

//Display
#define OLED_Address 0x3C //Indirizzo I2C del display OLED
#define SDA_pin     SDA
#define SCL_pin     SCL

//Time
#define timezone 2
#define refresh 5

//WebServer
#define WEBSERVER_ENABLED true
#define WEBSERVER_PORT  80
String themeColor = "blue-grey";

//File 
#define CONFIG "/conf.txt"

//Display
#define INVERT_DISPLAY false