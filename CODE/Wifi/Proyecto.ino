/*
    This sketch sends data via HTTP GET requests to data.sparkfun.com service.

    You need to get streamId and privateKey at data.sparkfun.com and paste them
    below. Or just customize this script to talk to other HTTP servers.

*/

#include <ESP8266WiFi.h>
#include <Wire.h>

//// BMP085: Librería para el barometro BMP085
//#include "Barometer.h"

// BMP180: Library to manage the BMP180 barometer
#include <SFE_BMP180.h>

// Variable declaration to manage the WIFI networt
const char* ssid     = "Hotspot";
const char* password = "%yatengowifi%";
// Use WiFiClient class to create TCP connections
WiFiClient client;
// Variable Setup
long lastConnectionTime = 0;
int failedCounter = 0;
boolean lastConnected     = false;
boolean uploadTemperature = false;

//const char* host = "data.sparkfun.com";
//const char* streamId   = "....................";
//const char* privateKey = "....................";

// Variable declaration to work with thingspeak
// ThingSpeak Settings
char thingSpeakAddress[] = "api.thingspeak.com";
String writeAPIKey_C = "7J5F3NW8FDLOJDX8";
// Time interval in milliseconds to update ThingSpeak (number of seconds * 1000 = interval)
const int updateThingSpeakInterval_C = 16 * 1000;      

//// BMP085: Variables para el barometro
//Barometer BMP085_BAROMETER;
////BMP085 variables:
//typedef struct BMP085_type
//{
//   float BMP085_T;
//   float BMP085_Pa;
//   float BMP085_ATM;
//   float BMP085_H;
//};
//BMP085_type BMP085_data;

// BMP180: Variable declaration for the BMP180 barometer.
//Se declara una instancia de la librería
SFE_BMP180 pressure;


typedef struct BMP180_type
{
   char    BMP180_status    = 0; // Whether the sensor give data or not.
   double  BMP180_Pa        = 0; // Read pressure in mBar.
   double  BMP180_H         = 0; // Elevation in m.
   double  BMP180_T         = 0; // Read Temperature ºC.
   double  BMP180_Initial_P = 0; // Reference pressure to calculate elevation
                                 // differences.
};
BMP180_type BMP180_data;


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

//   //Init the sensor
//   //BMP085 Initialise
//   BMP085_BAROMETER.init();

   //Initialize the sensor BMP180
//   Start_BMP180_Sensor();
  pinMode(BUILTIN_LED, OUTPUT); // Onboard LED  

}

//int value = 0;

//// Subprogram to read the BMP085 barometer
//BMP085_type Read_BMP085( )
//{
//   //Local variables
//   float atm;
//   BMP085_type BMP085_reading;
//
//   //Get the temperature, bmp085ReadUT MUST be called first
//   BMP085_reading.BMP085_T =
//      BMP085_BAROMETER.bmp085GetTemperature(BMP085_BAROMETER.bmp085ReadUT());
//
//   //Get the pressure
//   BMP085_reading.BMP085_Pa =
//      BMP085_BAROMETER.bmp085GetPressure(BMP085_BAROMETER.bmp085ReadUP());
//   BMP085_reading.BMP085_ATM = BMP085_reading.BMP085_Pa / 101325;
//
//   //Uncompensated caculation - in Meters
//   BMP085_reading.BMP085_H =
//      BMP085_BAROMETER.calcAltitude(BMP085_reading.BMP085_Pa);
//
//   // Write results
//   Serial.print(" BMP085 T: ");
//   Serial.print(BMP085_reading.BMP085_T, 2); //display 2 decimal places
//   Serial.print(" ºC  P: ");
//   Serial.print(BMP085_reading.BMP085_Pa, 0); //whole number only.
//   Serial.print(" Pa = ");
//   Serial.print(BMP085_reading.BMP085_ATM, 4); //display 4 decimal places
//   Serial.print("Atm.  Altitude: ");
//   Serial.print(BMP085_reading.BMP085_H, 2); //display 2 decimal places
//   Serial.println(" m");
//   Serial.println();
//
//   return BMP085_reading;
//
//};

void Start_BMP180_Sensor() {

   // Sensor initialization sequence.
   if (pressure.begin())
      Serial.println("BMP180 init success");
   else
   {

      Serial.println("BMP180 init fail (disconnected?)\n\n");
      while (1);
   }

   // Temperature initialization.
   BMP180_data.BMP180_status = pressure.startTemperature();
   if (BMP180_data.BMP180_status != 0)  {
      delay(BMP180_data.BMP180_status);
      //Initial Temperature.
      BMP180_data.BMP180_status = pressure.getTemperature(BMP180_data.BMP180_T);
      if (BMP180_data.BMP180_status != 0)    {
        // Pressure initialization.
         BMP180_data.BMP180_status = pressure.startPressure(3);
         if (BMP180_data.BMP180_status != 0)      {
            delay(BMP180_data.BMP180_status);
            //Initial Pressure in the sensor at initialization.
            BMP180_data.BMP180_status =
               pressure.getPressure
                  (BMP180_data.BMP180_Initial_P, BMP180_data.BMP180_T);
         }
         else Serial.println("Error 3. Presure is not initialised.\n");
      }
      else Serial.println("Error 2. Temperature is not available.\n");
   }
   else Serial.println("Error 1. Temperature is not initialised.\n");

   // Write results
   Serial.print(" BMP085 T: ");
   Serial.print(BMP180_data.BMP180_T, 2); //display 2 decimal places
   Serial.print(" ºC  Pres Inicial: ");
   Serial.print(BMP180_data.BMP180_Initial_P, 0); //whole number only.
   Serial.println();
}

// Subprogram to read the BMP085 barometer: Pressure, Temperature and Elevation
void Read_BMP180( ){
//   BMP180_data.BMP180_status = 0;
   Serial.println();
   Serial.println();
   Serial.println();
   Serial.println();
   Serial.println();
   Serial.println();
   Serial.println();
   Serial.println();
   Serial.println();
   Serial.println("*************************************** ");
   Serial.print("BMP180_data.BMP180_status:    ");
   Serial.println(BMP180_data.BMP180_status);
   Serial.println("*************************************** ");
//   Serial.print("status:    ");
//   Serial.println(status);
//   Serial.println("*************************************** ");

   if (BMP180_data.BMP180_status == 0){
      //Barometer sensor status is not available. Initialize the sensor BMP180
      Serial.print("Initialize the sensor BMP180");
      Start_BMP180_Sensor();

   }
   else 
   {

      Serial.println("Lectura de temperatura ");
      //Se inicia la lectura de temperatura
      BMP180_data.BMP180_status = pressure.startTemperature();
      if (BMP180_data.BMP180_status != 0)
      {
         delay(BMP180_data.BMP180_status);
         //Read Temperature.
         BMP180_data.BMP180_status = pressure.getTemperature(BMP180_data.BMP180_T);
         if (BMP180_data.BMP180_status != 0)
         {
            // Init Pressure read.
            BMP180_data.BMP180_status = pressure.startPressure(3);
            if (BMP180_data.BMP180_status != 0)
            {
               delay(BMP180_data.BMP180_status);

               // The read of the Pressure is affected by the temperature affecting
               // to the sensor readings).
               BMP180_data.BMP180_status =
                  pressure.getPressure(BMP180_data.BMP180_Pa, BMP180_data.BMP180_T);
               if (BMP180_data.BMP180_status != 0)
               {
                  //Elevation difference is calculated related to the initial Pressure.
                  BMP180_data.BMP180_H =
                     pressure.altitude
                        (BMP180_data.BMP180_Pa, BMP180_data.BMP180_Initial_P);
               }
               else Serial.println("Error en la lectura de presion\n");
            }
            else Serial.println("Error iniciando la lectura de presion\n");
         }
         else Serial.println("Error en la lectura de temperatura\n");
      }
      else Serial.println("Error iniciando la lectura de temperatura\n");
   }
   Serial.println("%%%%%%%%%%%%%%%%%%%%%%%%%%%%");
   Serial.print("BMP180_data.BMP180_status:    ");
   Serial.println(BMP180_data.BMP180_status);
   Serial.println("%%%%%%%%%%%%%%%%%%%%%%%%%%%");
}

//void updateThingSpeak(String tsData)
void updateThingSpeak(String tsData, String writeAPIKey)
{
   //Connect to wifi
   Serial.println("");
   Serial.println("");
   Serial.print("IP address: ");
   Serial.println(WiFi.localIP());
   Serial.print("Connecting to ");
   Serial.println(ssid);

   if (client.connect(thingSpeakAddress, 80))
   {
      client.print("POST /update HTTP/1.1\n");
      client.print("Host: api.thingspeak.com\n");
      client.print("Connection: close\n");
      client.print("X-THINGSPEAKAPIKEY: " + writeAPIKey + "\n");
      client.print("Content-Type: application/x-www-form-urlencoded\n");
      client.print("Content-Length: ");
      client.print(tsData.length());
      client.print("\n\n");
      client.print(tsData);
      lastConnectionTime = millis();
      delay(1000);
      if (client.connected())
      {
         Serial.println("Connecting to ThingSpeak...");
         Serial.println();
         failedCounter = 0;
      }
      else
      {
         failedCounter++;
         Serial.println("Connection to ThingSpeak failed (" + String(failedCounter, DEC) + ")");
         Serial.println();
      }
      delay(1000);
   }
   else
   {
      failedCounter++;
      Serial.println("Connection to ThingSpeak Failed (" + String(failedCounter, DEC) + ")");
      Serial.println();
      lastConnectionTime = millis();
   }
   //***************************************************************************
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


void loop() {

//   //Read the BMP085 barometer
//   BMP085_data = Read_BMP085();
//
//   // Write received data
//   Serial.print(" BMP085 T: ");
//   Serial.print(BMP085_data.BMP085_T, 2); //display 2 decimal places
//   Serial.print(" ºC  P: ");
//   Serial.print(BMP085_data.BMP085_Pa, 0); //whole number only.
//   Serial.print(" Pa = ");
//   Serial.print(BMP085_data.BMP085_ATM, 4); //display 4 decimal places
//   Serial.print("Atm.  Altitude: ");
//   Serial.print(BMP085_data.BMP085_H, 2); //display 2 decimal places
//   Serial.println(" m");
//   Serial.println();

   //Read the BMP180 barometer
   Read_BMP180();

   // Write received data
//   Serial.println(" ////// ");
   Serial.print(" BMP180 T: ");
   Serial.print(BMP180_data.BMP180_T, 2); //display 2 decimal places
   Serial.print(" ºC  P: ");
   Serial.print(BMP180_data.BMP180_Pa, 0); //whole number only.
   Serial.print(" mBar.  Altitude: ");
   Serial.print(BMP180_data.BMP180_H, 2); //display 2 decimal places
   Serial.println(" m");
   Serial.println();

   delay(5000);

//******************************************************************************
   // Update ThingSpeak
   if(!client.connected() && (millis() - lastConnectionTime > updateThingSpeakInterval_C))
   {
//      if (uploadTemperature == true)
      if (uploadTemperature == false)
      {
         // Upload the Temperatures.
         updateThingSpeak("field1="+String(BMP180_data.BMP180_T,DEC)
                         +"&field2="+String(BMP180_data.BMP180_Pa,DEC)
                         ,writeAPIKey_C);
  //***********************
  // Prueba para ver si funciona la subida alimentando con un cargador.
  //***********************
  // initialize digital pin BUILD_LED as an output.
  digitalWrite(BUILTIN_LED, LOW);   // turn off LED with voltage LOW
  Serial.println("");
  Serial.println("******************");
  Serial.println("** BLINK LOW *****");
  Serial.println("******************");
  Serial.println("");
  delay(1000);                      // wait one second
  digitalWrite(BUILTIN_LED, HIGH);  // turn on LED with voltage HIGH
  Serial.println("");
  Serial.println("******************");
  Serial.println("** BLINK HIGH ****");
  Serial.println("******************");
  Serial.println("");
  delay(1000);                      // wait one second
//  Serial.println(BUILTIN_LED);



//         // Reset the variable.
//         uploadTemperature = false;
//      }
//      else
//      {
//         // Upload the Monitor data.
//         updateThingSpeak("field1="+String(contadorciclo,DEC)
//                        +"&field2="+String(Rlight,DEC)
//                        +"&field3="+String(sumUV,DEC),writeAPIKeyM);
//         // Change variable value.
//         uploadTemperature = true;
      }
   }
   // Check if Arduino Wifi needs to be restarted
   if (failedCounter > 3 )
   {
      WiFi.begin(ssid, password);
   }

   lastConnected = client.connected();
//******************************************************************************

//   //Connect to wifi
//   ++value;
//   Serial.println("");
//   Serial.println("");
//   Serial.print("IP address: ");
//   Serial.println(WiFi.localIP());
//   Serial.print("Connecting to ");
//   Serial.println(host);
//
//   // Use WiFiClient class to create TCP connections
//   WiFiClient client;
//   const int httpPort = 80;
//   if (!client.connect(host, httpPort)) {
//     Serial.println("connection failed");
//     return;
//   }
//
//   // We now create a URI for the request
//   String url = "/input/";
//   url += streamId;
//   url += "?private_key=";
//   url += privateKey;
//   url += "&value=";
//   url += value;
//
//   Serial.print("Requesting URL: ");
//   Serial.println(url);
//
//   // This will send the request to the server
//   client.print(String("GET ") + url + " HTTP/1.1\r\n" +
//                "Host: " + host + "\r\n" +
//                "Connection: close\r\n\r\n");
//   int timeout = millis() + 5000;
//   while (client.available() == 0) {
//     if (timeout - millis() < 0) {
//       Serial.println(">>> Client Timeout !");
//       client.stop();
//       return;
//     }
//   }
//
//   // Read all the lines of the reply from server and print them to Serial
//   while (client.available()) {
//     String line = client.readStringUntil('\r');
//     Serial.print(line);
//   }
//
//   Serial.println("closing connection");
//   Serial.println();
//   Serial.println();
}

