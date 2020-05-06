#include "time.h"
#include "ESP8266WiFi.h"
#include "TridentTD_LineNotify.h"
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)
#define LINE_TOKEN "" // แก้ Line Token

Adafruit_BME280 bme; // I2C

ESP8266WebServer server(80);

// Hotspot@HOME
// const char* ssid     = "";                    // your network SSID (name)
// const char* password = "";                    // your network password

// Hotspot@WORK
const char* ssid     = "";                       // your network SSID (name)
const char* password = "";                       // your network password

// Pressure param @ VTSE
const double  Elevation = 1550.75;  // centimeter  , 13.657 m.
double  Elevation_feet = Elevation/30;  // feet
double  Elevation_QFE = Elevation_feet/30;
double  ranway = 500;  // cm. : runway serface diff msl
double heat_circuit = 0.15; // Heat from Circuit
//const double  baro_error = -0.07;  // mb.

double h = 0.0;
double t = 0.0; // อุณหภูมิ BMP280
double QNH = 0.0; // ความกด BME280
double QFE_VTSE = 0.0;
double QFE_VTSE_inhg = 0.0;
double a = 0.0;

// Time param
const char* ntpServer = "ntp.ku.ac.th";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 7*3600;

const long min_to_post_a = 10;           // Setting time for send data in min
const long sec_to_post_a = 15;
const long min_to_post_b = 20;           // Setting time for send data in min
const long sec_to_post_b = 15;
const long min_to_post_c = 30;           // Setting time for send data in min
const long sec_to_post_c = 15;
const long min_to_post_d = 40;           // Setting time for send data in min
const long sec_to_post_d = 15;
const long min_to_post_e = 50;           // Setting time for send data in min
const long sec_to_post_e = 15;
const long min_to_post_f = 00;           // Setting time for send data in min
const long sec_to_post_f = 15;

bool time_trigger = false;              // Trigger for send data

char buffer[80];

// Function for Print time
void printLocalTime()
{
  time_t rawtime;
  struct tm * timeinfo;
  time (&rawtime);
  timeinfo = localtime (&rawtime);
 
  strftime (buffer,80,"%d %B %Y \nUTC+7 @ %H:%M \n",timeinfo); //:%S
  
  if(timeinfo->tm_min == min_to_post_a && timeinfo->tm_sec == sec_to_post_a){   //  timeinfo->tm_min == min_to_post &&
    time_trigger = true;
    Serial.println(time_trigger);
    Serial.println(buffer);
    //break;
  }else if(timeinfo->tm_min == min_to_post_b && timeinfo->tm_sec == sec_to_post_b){   //  timeinfo->tm_min == min_to_post && timeinfo->tm_sec == sec_to_post_b
    time_trigger = true;
    Serial.println(time_trigger);
    Serial.println(buffer);
    //break;
  }else if(timeinfo->tm_min == min_to_post_c && timeinfo->tm_sec == sec_to_post_c){   //  timeinfo->tm_min == min_to_post &&
    time_trigger = true;
    Serial.println(time_trigger);
    Serial.println(buffer);
    //break;
  }else if(timeinfo->tm_min == min_to_post_d && timeinfo->tm_sec == sec_to_post_d){   //  timeinfo->tm_min == min_to_post &&
    time_trigger = true;
    Serial.println(time_trigger);
    Serial.println(buffer);
    //break;
  }else if(timeinfo->tm_min == min_to_post_e && timeinfo->tm_sec == sec_to_post_e){   //  timeinfo->tm_min == min_to_post &&
    time_trigger = true;
    Serial.println(time_trigger);
    Serial.println(buffer);
    //break;
  }else if(timeinfo->tm_min == min_to_post_f && timeinfo->tm_sec == sec_to_post_f){   //  timeinfo->tm_min == min_to_post &&
    time_trigger = true;
    Serial.println(time_trigger);
    Serial.println(buffer);
    //break;
  }
  
}


//----------------------
// SETUP SECTION : START
//----------------------
void setup() 
{
  Serial.begin(115200);  
  delay(100);

  // Start BME280
  // bme.begin(0x76);
  bool status = bme.begin(0x76); 
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  // We start by connecting to a WiFi network
  Serial.print("\n\nConnecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  String ipBuff = WiFi.localIP().toString();
  // Start HTTP Server
  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");
  
  // Connect to Line API
  LINE.setToken(LINE_TOKEN);
  LINE.notify(" VTSE START\nServer IP : " + ipBuff);

  // Connect to Time Server
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("\nWaiting for time");
  unsigned timeout = 5000;
  unsigned start = millis();
  while (!time(nullptr)) 
  {
    Serial.print(".");
    delay(1000);
  }
  delay(1000);
  //Serial.println("Time...");
}
//---------------------
// SETUP SECTION : END
//---------------------



//----------------------------
// HTTP SERVER SECTION : START
//----------------------------
void handle_OnConnect() {
  server.send(200, "text/html", SendHTML(t, h, QNH, QFE_VTSE, QFE_VTSE_inhg,a)); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML(double temperature,double humidity,double QNH, double QFE, double QFE_inhg, double a){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>VTSE Weather Station</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
  ptr +="p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<div id=\"webpage\">\n";
  ptr +="<h1>VTSE Weather Station</h1>\n";
  ptr +="<p>QNH : ";
  ptr +=QNH;
  ptr +=" hPa</p>";
  
  ptr +="<p>QFE : ";
  ptr +=QFE;
  ptr +=" hPa</p>";
  
  ptr +="<p>QFE in : ";
  ptr +=QFE_inhg;
  ptr +=" inHg</p>";
  
  ptr +="<p>Temperature: ";
  ptr +=temperature;
  ptr +=" &deg;C</p>";
  ptr +="<p>Humidity: ";
  ptr +=humidity;
  ptr +=" %</p>";
  
  //ptr +="<p>Altitude: ";
  //ptr +=a;
  //ptr +=" m.</p>";
  ptr +="</div>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}
//--------------------------
// HTTP SERVER SECTION : END
//--------------------------



//---------------------
// MAIN PROGRAM : START
//---------------------
void loop() 
{
  server.handleClient();  
  printLocalTime(); // Check time to print 
  
      
      String val = "VTSE OBSERVER\n";
      h = bme.readHumidity();
      t = bme.readTemperature(); // อุณหภูมิ BMP280
      t -= heat_circuit ; // decreast by temp in circuit
      QNH = bme.readPressure() / 100.0F; // ความกด BME280
      
      //a = bme.readAltitude(SEALEVELPRESSURE_HPA); // ความสูง BMP280
      QFE_VTSE = QNH - Elevation_QFE;
      QFE_VTSE = QFE_VTSE+(ranway/900)-0.25;
      //QFE_VTSE_inhg = QFE_VTSE/33.864;
      QFE_VTSE_inhg = QFE_VTSE*100*0.0002953;
      val = val + buffer +"\n";
      
      val = val +"QNH  [hPa]   " + QNH +"\n";
      val = val +"QFE  [hPa]   " + QFE_VTSE +"\n";
      val = val +"QFE' [inHg]  " + QFE_VTSE_inhg +"\n\n";     
      
      val = val +"Temp  [C]   "+ t+"\n";
      val = val +"Humid [%]   "+ h+"\n\n";
      
      //val = val +"Alti  [m]   " + a +"\n";

  // SEND DATA TO SERVER
  delay(1000);
  server.handleClient();

  
  if(time_trigger == true){
      Serial.println(val);
      LINE.notify(val); 
      delay(1000);  
      time_trigger = false;     
  } 
 
}

//-------------------
// MAIN PROGRAM : END
//-------------------
