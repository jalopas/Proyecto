/*
    This sketch sends data via HTTP GET requests to data.sparkfun.com service.

    You need to get streamId and privateKey at data.sparkfun.com and paste them
    below. Or just customize this script to talk to other HTTP servers.

*/

#include <ESP8266WiFi.h>
#include <Wire.h>

// BMP085: Librería para el barometro BMP085
#include "Barometer.h"

const char* ssid     = "Hotspot";
const char* password = "%yatengowifi%";

const char* host = "data.sparkfun.com";
const char* streamId   = "....................";
const char* privateKey = "....................";
// Variable declaration to work with thingspeak
// ThingSpeak Settings
char thingSpeakAddress[] = "api.thingspeak.com";
String writeAPIKey = "7J5F3NW8FDLOJDX8";

// BMP085: Variables para el barometro 
Barometer BMP085_BAROMETER;
//BMP085 variables:
typedef struct BMP085_type
  {
     float BMP085_T;
     float BMP085_Pa;
     float BMP085_ATM;
     float BMP085_H;
  };
BMP085_type BMP085_data;
  
void setup() {
  //Serial.begin(115200);
  Serial.begin(9600);
  delay(10);

  // We start by connecting to a WiFi network
  // Se comienza conectando a la WiFi
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  // Repeat until the conexion is established
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //The Wifi connection has been opened. 
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  //Init the sensor
  //BMP085 Initialise
  BMP085_BAROMETER.init();

}
int value = 0;
// Subprogram to read the BMP085 barometer
BMP085_type Read_BMP085( )
{
   //Local variables
   float atm;
   BMP085_type BMP085_reading;

   //Get the temperature, bmp085ReadUT MUST be called first
   BMP085_reading.BMP085_T = BMP085_BAROMETER.bmp085GetTemperature(BMP085_BAROMETER.bmp085ReadUT());
   
   //Get the presure
   BMP085_reading.BMP085_Pa = BMP085_BAROMETER.bmp085GetPressure(BMP085_BAROMETER.bmp085ReadUP());
   BMP085_reading.BMP085_ATM = BMP085_data.BMP085_Pa / 101325;
   
   //Uncompensated caculation - in Meters
   BMP085_reading.BMP085_H = BMP085_BAROMETER.calcAltitude(BMP085_reading.BMP085_Pa); 
   
   // Write results
   Serial.print(" BMP085 T: ");
   Serial.print(BMP085_reading.BMP085_T, 2); //display 2 decimal places
   Serial.print(" ºC  P: ");
   Serial.print(BMP085_reading.BMP085_Pa, 0); //whole number only.
   Serial.print(" Pa = ");
   Serial.print(BMP085_reading.BMP085_ATM, 4); //display 4 decimal places
   Serial.print("Altitude: ");
   Serial.print(BMP085_reading.BMP085_H, 2); //display 2 decimal places
   Serial.println(" m");
   Serial.println();
   
   return BMP085_reading;
   
};

void loop() {

  //Read the BMP085 barometer
  Read_BMP085();
  // Write data
  Serial.print(" BMP085 T: ");
  Serial.print(BMP085_data.BMP085_T, 2); //display 2 decimal places
  Serial.print(" ºC  P: ");
  Serial.print(BMP085_data.BMP085_Pa, 0); //whole number only.
  Serial.print(" Pa = ");
  Serial.print(BMP085_data.BMP085_ATM, 4); //display 4 decimal places
  Serial.print("Altitude: ");
  Serial.print(BMP085_data.BMP085_H, 2); //display 2 decimal places
  Serial.println(" m");
  Serial.println();

  delay(5000);

  //Connect to wifi
  ++value;
  Serial.println("");
  Serial.println("");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // We now create a URI for the request
  String url = "/input/";
  url += streamId;
  url += "?private_key=";
  url += privateKey;
  url += "&value=";
  url += value;

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  int timeout = millis() + 5000;
  while (client.available() == 0) {
    if (timeout - millis() < 0) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println("closing connection");
  Serial.println();
  Serial.println();
}

