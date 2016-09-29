/*
    This sketch sends data via HTTP GET requests to data.sparkfun.com service.

    You need to get streamId and privateKey at data.sparkfun.com and paste them
    below. Or just customize this script to talk to other HTTP servers.
IOT:
https://www.pushingbox.com/
https://www.carriots.com/tutorials/others_APIs/google_spreadsheets
http://www.instructables.com/id/Post-to-Google-Docs-with-Arduino/
https://docs.google.com/forms/d/1cRJwf9MiV4jKkmAiYpPgx1Uzce4K-DZQ2v0l0ywWUZY/viewform
https://docs.google.com/forms/d/1cRJwf9MiV4jKkmAiYpPgx1Uzce4K-DZQ2v0l0ywWUZY/formResponse
http://www.electroschematics.com/11291/arduino-dht22-am2302-tutorial-library/
https://productforums.google.com/forum/#!msg/docs/f4hJKF1OQOw/9k2oJ-kLULsJ
https://www.pushingbox.com/dashboard.php
Particle Sensor:
http://www.elecrow.com/wiki/index.php?title=Dust_Sensor-_DSM501A
http://www.samyoungsnc.com/products/3-1%20Specification%20DSM501.pdf
https://publiclab.org/system/images/photos/000/003/726/original/tmp_DSM501A_Dust_Sensor630081629.pdf
*/
#include <ESP8266WiFi.h>
#include <Wire.h>

//// BMP085: Librer?a para el barometro BMP085
//#include "Barometer.h"

// BMP180: Library to manage the BMP180 barometer
#include <SFE_BMP180.h>

// Variable declaration to manage the WIFI networt
const char* WIFI_ssid     = "Hotspot";
const char* password = "%yatengowifi%";
// Use WiFiClient class to create TCP connections
WiFiClient client;
// Variable Setup
long lastConnectionTime = 0;
int failedCounter = 0;
boolean lastConnected     = false;
int uploadCounter = 1;
int blinkCounter = 1;

// Time interval in milliseconds to update ThingSpeak (number of seconds * 1000 = interval)
const int updateThingSpeakInterval_C = 16 * 1000;

// BMP180: Variable declaration for the BMP180 barometer.
//Se declara una instancia de la librer?a
SFE_BMP180 pressure;


typedef struct BMP180_type
{
   char    STATUS      = 0; // Whether the sensor give data or not.
   double  P_mBa       = 0; // Read pressure in mBar.
   double  HIGH_m      = 0; // Elevation in m.
   double  TEMPERATURE = 0; // Read Temperature ?C.
   double  P_INI       = 0; // Reference pressure to calculate elevation differences.
};
BMP180_type BMP180_data;

// DSM501A: Variable declaration for the DSM501A particle sensor.
// Connect the Pin_3 of DSM501A to Arduino 5V
// Connect the Pin_5 of DSM501A to Arduino GND
// Connect the Pin_2 of DSM501A to Arduino D8
// Define pines https://community.thinger.io/t/esp8266-with-arduino-ide/20

typedef struct DSM501A_type
{
  // byte buff[2];
   unsigned long DURATION;
   unsigned long START_TIME;
   unsigned long END_TIME;
   unsigned long SAMPLE_TIME         = 30000; // sample 30 s
   unsigned long LOW_PULSE_OCCUPANCY = 0;
   float         RATIO               = 0;
   float         CONCENTRATION       = 0;
};
DSM501A_type DSM501A_data;

void setup() {
   //Serial.begin(115200);
   Serial.begin(9600);
   delay(10);

   //Initialize the sensor BMP180
   //   Start_BMP180_Sensor();
   // Declare LED pin;
   pinMode(D8, INPUT);
   pinMode(BUILTIN_LED, OUTPUT); // Onboard LED
   // Declare pin for DSM501A particle sensor;

   DSM501A_data.START_TIME = millis(); 

   // We start by connecting to a WiFi network
   // Se comienza conectando a la WiFi
   Serial.println();
   Serial.println();
   Serial.print("Connecting to ");
   Serial.println(WIFI_ssid);

   WiFi.begin(WIFI_ssid, password);

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

}

// Rutina para leer el sensor de polvo.
void Read_DSM501A() {
  //http://www.elecrow.com/wiki/index.php?title=Dust_Sensor-_DSM501A
  DSM501A_data.END_TIME = millis();
  delay (5000);
  if (( DSM501A_data.END_TIME - DSM501A_data.START_TIME) > DSM501A_data.SAMPLE_TIME)
  {
     DSM501A_data.DURATION = pulseIn(D8, LOW);
     DSM501A_data.LOW_PULSE_OCCUPANCY += DSM501A_data.DURATION;

    DSM501A_data.RATIO = ( DSM501A_data.LOW_PULSE_OCCUPANCY-
                         DSM501A_data.END_TIME+
               DSM501A_data.START_TIME + 
               DSM501A_data.SAMPLE_TIME)
               /
             ( DSM501A_data.SAMPLE_TIME*10.0 );  // Integer percentage 0=>100
    DSM501A_data.CONCENTRATION = 1.1*pow(DSM501A_data.RATIO,3)-3.8*pow(DSM501A_data.RATIO,2)+520*DSM501A_data.RATIO+0.62; // using spec sheet curve
    Serial.print("lowpulseoccupancy:");
    Serial.print(DSM501A_data.LOW_PULSE_OCCUPANCY);
    Serial.print("    ratio:");
    Serial.print(DSM501A_data.RATIO);
    Serial.print("    DSM501A:");
    Serial.println(DSM501A_data.CONCENTRATION);
    DSM501A_data.LOW_PULSE_OCCUPANCY = 0;
    DSM501A_data.START_TIME = millis();
  } 
}
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
   BMP180_data.STATUS = pressure.startTemperature();
   if (BMP180_data.STATUS != 0)  {
      delay(BMP180_data.STATUS);
      //Initial Temperature.
      BMP180_data.STATUS = pressure.getTemperature(BMP180_data.TEMPERATURE);
      if (BMP180_data.STATUS != 0)    {
        // Pressure initialization.
         BMP180_data.STATUS = pressure.startPressure(3);
         if (BMP180_data.STATUS != 0)      {
            delay(BMP180_data.STATUS);
            //Initial Pressure in the sensor at initialization.
            BMP180_data.STATUS =
               pressure.getPressure
                  (BMP180_data.P_INI, BMP180_data.TEMPERATURE);
         }
         else Serial.println("Error 3. Presure is not initialised.\n");
      }
      else Serial.println("Error 2. Temperature is not available.\n");
   }
   else Serial.println("Error 1. Temperature is not initialised.\n");
/*
   // Write results
   Serial.print(" BMP085 T: ");
   Serial.print(BMP180_data.TEMPERATURE, 2); //display 2 decimal places
   Serial.print(" C  Pres Inicial: ");
   Serial.print(BMP180_data.P_INI, 0); //whole number only.
   Serial.println();
*/
}

// Subprogram to read the BMP085 barometer: Pressure, Temperature and Elevation
void Read_BMP180( ){
 
   if (BMP180_data.STATUS == 0){
      //Barometer sensor status is not available. Initialize the sensor BMP180
      //Serial.println("Initialize the sensor BMP180");
      Start_BMP180_Sensor();

   }
   else
   {

      //Se inicia la lectura de temperatura
      BMP180_data.STATUS = pressure.startTemperature();
      if (BMP180_data.STATUS != 0)
      {
         delay(BMP180_data.STATUS);
         //Read Temperature.
         BMP180_data.STATUS = pressure.getTemperature(BMP180_data.TEMPERATURE);
         if (BMP180_data.STATUS != 0)
         {
            // Init Pressure read.
            BMP180_data.STATUS = pressure.startPressure(3);
            if (BMP180_data.STATUS != 0)
            {
               delay(BMP180_data.STATUS);

               // The read of the Pressure is affected by the temperature affecting
               // to the sensor readings).
               BMP180_data.STATUS =
                  pressure.getPressure(BMP180_data.P_mBa, BMP180_data.TEMPERATURE);
               if (BMP180_data.STATUS != 0)
               {
                  //Elevation difference is calculated related to the initial Pressure.
                  BMP180_data.HIGH_m =
                     pressure.altitude
                        (BMP180_data.P_mBa, BMP180_data.P_INI);
               }
               else Serial.println("Error en la lectura de presion\n");
            }
            else Serial.println("Error iniciando la lectura de presion\n");
         }
         else Serial.println("Error en la lectura de temperatura\n");
      }
      else Serial.println("Error iniciando la lectura de temperatura\n");
   }
}

// Send stream to Carriots
// Send stream to Carriots
void updateCarriots()
{
   // Variable declaration to work with Carriots

  const String Carriots_APIKEY = "5917b4cb109c087d1707f6179cc5593f6656c663f8a897469601632488c46611"; // Carriots apikey
  const String DEVICE = "ProyectoArduino@pinfocal.pinfocal"; // Replace with the id_developer of your device
  IPAddress carriotsAddress(82,223,244,60);  // api.carriots.com IP Address

   Serial.println("");
   Serial.println("");
   Serial.print(" Carriots: ");
   Serial.print(WIFI_ssid);
   Serial.print(" IP address: ");
   Serial.println(WiFi.localIP());

  if (client.connect(carriotsAddress, 80)) 
  {  // If there's a successful connection
     // Build the data field
     String json = "{\"protocol\":\"v2\",\"device\":\""+DEVICE+"\",\"at\":\"now\",\"data\":{\"Presion\":\""+BMP180_data.TEMPERATURE+
                                                                                        "\",\"Presion2\":\""+BMP180_data.TEMPERATURE+
                                                                                        "\",\"Presion3\":\""+BMP180_data.TEMPERATURE+"\"}}";
    // Make a HTTP request
    client.println("POST /streams HTTP/1.1");
    client.println("Host: api.carriots.com");
    client.println("Accept: application/json");
    client.println("User-Agent: Arduino-Carriots");
    client.println("Content-Type: application/json");
    client.print("carriots.apikey: ");
    client.println(Carriots_APIKEY);
    client.print("Content-Length: ");
    client.println(json.length());
    client.println(json);
    client.println("Connection: close");
    client.println();
    client.println(json);

      lastConnectionTime = millis();
      delay(1000);
      if (client.connected())
      {
 /*        Serial.println("Connecting to Carriots...");
         Serial.println();
*/
         failedCounter = 0;
      }
      else
      {
         failedCounter++;
         Serial.println("Error_1: Connection to Carriots failed (" + String(failedCounter, DEC) + ")");
         Serial.println();
      }
      delay(1000);

  }
  else 
  {
      failedCounter++;
      Serial.println("Error_2: Connection to Carriots Failed (" + String(failedCounter, DEC) + ")");
      Serial.println();
      lastConnectionTime = millis();
  }
   //***************************************************************************
   int timeout = millis() + 5000;

   while (client.available() == 0) 
   {
      if (timeout - millis() < 0) 
      {
         Serial.println(">>> Client Timeout !");
         client.stop();
         return;
      }
   }

   // Read all the lines of the reply from server and print them to Serial
   while (client.available()) 
   {
      String line = client.readStringUntil('\r');
      Serial.print(line);
   }

   Serial.println("closing connection");
   Serial.println();
   Serial.println();
} // end updateCarriots

void updatePushingbox()
{
   char pushingbox_Address[] = "api.pushingbox.com";
   char pushingbox_ID[] = "vC6A8513890F3CEF"; // THIS IS THE DEVICE ID FROM PUSHINGBOX
   char pushingbox_msg[100];

   //Connect to wifi
   Serial.println("");
   Serial.println("");
   Serial.print(" Pushingbox: ");
   Serial.print(WIFI_ssid);
   Serial.print(" IP address: ");
   Serial.println(WiFi.localIP());

   if (client.connect(pushingbox_Address, 80))
   {
/* Codigo del get
      Serial.println("connected");
      static char CHAR_BMP180_T[15];
      dtostrf(BMP180_data.TEMPERATURE,5, 2, CHAR_BMP180_T); // Pasa el double a char.
      sprintf(pushingbox_msg,"GET /pushingbox?devid=%s&status=%s HTTP/1.1",pushingbox_ID,CHAR_BMP180_T);
      client.println(pushingbox_msg);
      client.println("Host: api.pushingbox.com");
      client.println("Connection: close");
      client.println();
      Serial.println(pushingbox_msg);
      Serial.println("Host: api.pushingbox.com");
      Serial.println("Connection: close");
      Serial.println();
*/
//*********************************************
String data="";
char str1[10];   
char str2[10];  
char str3[10];  
char str4[10];  
char str5[10]; 
char str6[10];
char str7[10];
char str8[10];
char str9[10];
char str10[10]; 
      Serial.println("........1..");

  data="";
  data+="";
  data+="&Temperatura=";
      static char CHAR_BMP180_T[10];
      dtostrf(BMP180_data.TEMPERATURE,5, 2, CHAR_BMP180_T); // Pasa el double a char.
  data+=CHAR_BMP180_T;     
  data+="&Presion=";
      static char CHAR_P_mBa[10];
      dtostrf(BMP180_data.P_mBa,5, 2, CHAR_P_mBa); // Pasa el double a char.
  data+=CHAR_P_mBa;
  data+="&Altitud=";
      static char CHAR_HIGH[10];
  dtostrf(BMP180_data.HIGH_m,4,2,CHAR_HIGH);
  data+=CHAR_HIGH; 

  data+="&status=25";
/*  dtostrf(InHum,4,2,str4);
  data+=str4;

 data+="&BMPTemp=";
  dtostrf(BMPTemp,4,2,str5);
  data+=str5;    
  data+="&BarometricPress=";
  dtostrf(BarometerPress,4,2,str6);
  data+=str6;
 data+="&CrawlTemp=";
  dtostrf(CrawlTemp,4,2,str7);
  data+=str7; 
  data+="&CrawlHum=";
  dtostrf(CrawlHum,4,2,str8);
  data+=str8;

 data+="&OutTemp=";
  dtostrf(OutTemp,4,2,str9);
  data+=str9; 
  data+="&OutHum=";
  dtostrf(OutHum,4,2,str10);
  data+=str10;
*/
  data+= "&&submit=Submit";
  
    Serial.println ("connecting ...");
    Serial.print ("data ...");
    Serial.println (data);
    
  // Send the HTTP PUT request:
    client.print("POST /pushingbox/pushingbox?devid=");
    client.print(pushingbox_ID);
    client.println(" HTTP/1.1");
    client.println("Host: api.pushingbox.com");
    client.println("Content-Type: application/x-www-form-urlencoded");  
    client.println("Connection: close");
    client.print("Content-Length: ");
    client.println(data.length());
    client.println();
    client.print(data);
    client.println();
//    delay(100);
//    client.stop(); 
/*
      Serial.println("connected");
      static char CHAR_BMP180_T[15];
      dtostrf(BMP180_data.TEMPERATURE,5, 2, CHAR_BMP180_T); // Pasa el double a char.
      sprintf(pushingbox_msg,"GET /pushingbox?devid=%s&status=%s HTTP/1.1",pushingbox_ID,CHAR_BMP180_T);
      client.println(pushingbox_msg);
      client.println("Host: api.pushingbox.com");
      client.println("Connection: close");
      client.println();
      Serial.println(pushingbox_msg);
      Serial.println("Host: api.pushingbox.com");
      Serial.println("Connection: close");
      Serial.println();
*/

//*********************************************
 
      delay(1000);
      lastConnectionTime = millis();
      delay(1000);
      if (client.connected())
      {
         Serial.println("Connecting to Pushingbox ...");
         Serial.println();
         failedCounter = 0;
      }
      else
      {
         failedCounter++;
         Serial.println("Error_3: Connection to Pushingbox failed (" + String(failedCounter, DEC) + ")");
         Serial.println();
      }
      delay(1000);
    }
    else
    {
       failedCounter++;
       Serial.println("Error_4: Connection to Pushingbox Failed (" + String(failedCounter, DEC) + ")");
       Serial.println();
       lastConnectionTime = millis();
    }
    //***************************************************************************
    int timeout = millis() + 5000;
    while (client.available() == 0) 
    {
       if (timeout - millis() < 0) 
       {
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
} //end updatePushingbox;



//void updateThingSpeak(String tsData)
//void updateThingSpeak(String tsData, String writeAPIKey)
void updateThingSpeak()
{
   // Variable declaration to work with thingspeak
   // ThingSpeak Settings
   char thingSpeakAddress[] = "api.thingspeak.com";
   String writeAPIKey_C = "7J5F3NW8FDLOJDX8";
   // Google forms Settings
   char Google_form_Address[] = "www.google.es";
   char Google_form_key[] = "1cRJwf9MiV4jKkmAiYpPgx1Uzce4K-DZQ2v0l0ywWUZY"; //Replace with your Key
   //https://docs.google.com/forms/d/1cRJwf9MiV4jKkmAiYpPgx1Uzce4K-DZQ2v0l0ywWUZY/e
   //byte Google_form_Address[] = { 209,85,229,101 }; // Google IP
   //byte Google_form_Address[] = { 216,58,209,163 }; // Google IP Alemania
   //Connect to wifi
   Serial.println("");
   Serial.println("");
   Serial.print(" ThingSpeak: ");
   Serial.print(WIFI_ssid);
   Serial.print(" IP address: ");
   Serial.println(WiFi.localIP());

   if (client.connect(thingSpeakAddress, 80))
   {
      // Build the data field
      String tsData = "field1="+String(BMP180_data.TEMPERATURE,DEC)
                    +"&field2="+String(BMP180_data.P_mBa,DEC);

      // Make a HTTP request
      client.print("POST /update HTTP/1.1\n");
      client.print("Host: api.thingspeak.com\n");
      client.print("Connection: close\n");
      client.print("X-THINGSPEAKAPIKEY: " + writeAPIKey_C + "\n");
      client.print("Content-Type: application/x-www-form-urlencoded\n");
      client.print("Content-Length: ");
      client.print(tsData.length());
      client.print("\n\n");
      client.print(tsData);
      lastConnectionTime = millis();
      delay(1000);
      if (client.connected())
      {
 /*        Serial.println("Connecting to ThingSpeak...");
         Serial.println();
*/
         failedCounter = 0;
      }
      else
      {
         failedCounter++;
         Serial.println("Error_5: Connection to ThingSpeak failed (" + String(failedCounter, DEC) + ")");
         Serial.println();
      }
      delay(1000);
   }
   else
   {
      failedCounter++;
      Serial.println("Error_6: Connection to ThingSpeak Failed (" + String(failedCounter, DEC) + ")");
      Serial.println();
      lastConnectionTime = millis();
   }
   //***************************************************************************
   int timeout = millis() + 5000;

   while (client.available() == 0) 
   {
      if (timeout - millis() < 0) 
      {
         Serial.println(">>> Client Timeout !");
         client.stop();
         return;
      }
   }

   // Read all the lines of the reply from server and print them to Serial
   while (client.available()) 
   {
      String line = client.readStringUntil('\r');
      Serial.print(line);
   }

   Serial.println("closing connection");
   Serial.println();
   Serial.println();
}  //updateThingSpeak


void loop() {

  // Sensors are only read when the arduino is not uploading data to the network.
  if (!client.connected())
  {

      //Read the BMP180 barometer
      Read_BMP180();
      // Write received data
      Serial.print(" BMP180 T: ");
      Serial.print(BMP180_data.TEMPERATURE, 2); //display 2 decimal places
      Serial.print(" ?C  P: ");
      Serial.print(BMP180_data.P_mBa, 0); //whole number only.
      Serial.print(" mBar.  Altitude: ");
      Serial.print(BMP180_data.HIGH_m, 2); //display 2 decimal places
      Serial.print(" m");
//   Serial.println();
      delay(5000);
      Read_DSM501A();
//******************************************************************************
     Serial.print(" client no connected");
//     if(!client.connected()){Serial.print(" no connected");}else{Serial.print(" conected");}
     Serial.print(" millis");
     Serial.print(millis());
     Serial.print(" lastConnectionTime");
     Serial.print(lastConnectionTime);
     Serial.println();
  }
  else
  {
     Serial.println(" client connected");	  
  }
   // Update data to the IoT server
   if(!client.connected() && (millis() - lastConnectionTime > updateThingSpeakInterval_C))
   {
      //Set the number of blinks counter.
      blinkCounter = uploadCounter;
   
      Serial.print(" uploadCounter: ");       
      Serial.println(uploadCounter);
       
      switch (uploadCounter) 
      {
         case 1:
            // Upload the Temperatures.
/*
            updateThingSpeak("field1="+String(BMP180_data.TEMPERATURE,DEC)
                            +"&field2="+String(BMP180_data.P_mBa,DEC)
                            ,writeAPIKey_C);
*/
            updateThingSpeak();
            // Reset the variable.
            uploadCounter = 2;
            break;

         case 2:
            // Upload the Monitor data.
            updatePushingbox();   
 /*        updateGoogleForms("entry.0.single="+String(BMP180_data.TEMPERATURE,DEC)
                        +"&entry.2.single="+String(BMP180_data.P_mBa,DEC)
                        +"&submit=Submit");
 */
            // Change variable value.
            uploadCounter = 3;
            break;

         case 3:
            // Upload the Monitor data.
            updateCarriots();
            // Change variable value.
            uploadCounter = 1;
            break;
     
         default: 
            // if nothing else matches, do the default
            // default is optional
            uploadCounter = 1;
            break;
      } 

      for (int i=0; i <= blinkCounter; i++){

         // initialize digital pin BUILD_LED as an output.
         //  Serial.println("** BLINK LOW *****");
         digitalWrite(BUILTIN_LED, LOW);   // turn on LED with voltage LOW
         delay(1000);                      // wait one second
         //  Serial.println("** BLINK HIGH ****");
         digitalWrite(BUILTIN_LED, HIGH);  // turn off LED with voltage HIGH
         delay(1000);                      // wait one second
      } 
   }
   // Check if Arduino Wifi needs to be restarted
   if (failedCounter > 3 )
   {
      WiFi.begin(WIFI_ssid, password);
   }

   lastConnected = client.connected();
}

  