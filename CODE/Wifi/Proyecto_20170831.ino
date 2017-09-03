/*
    This sketch sends data via HTTP GET requests to data.sparkfun.com service.

    You need to get streamId and privateKey at data.sparkfun.com and paste them
    below. Or just customize this script to talk to other HTTP servers.
IOT:
PUSHINGBOX
https://www.pushingbox.com/
http://www.instructables.com/id/Post-to-Google-Docs-with-Arduino/
https://docs.google.com/forms/d/1cRJwf9MiV4jKkmAiYpPgx1Uzce4K-DZQ2v0l0ywWUZY/viewform
https://docs.google.com/forms/d/1cRJwf9MiV4jKkmAiYpPgx1Uzce4K-DZQ2v0l0ywWUZY/formResponse

CARRIOTS
https://www.carriots.com/tutorials/others_APIs/google_spreadsheets

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
#include "MutichannelGasSensor.h"

// BMP180: Library to manage the BMP180 barometer
#include <SFE_BMP180.h>

// Library to convert float to string
//#include "floatToString.h" 

// Biblioteca para mandar, visualizar y analizar cadenas de datos en la nube.
#include "ThingSpeak.h"

// Variable declaration to manage the WIFI networt
const char* WIFI_ssid = "Hotspot";
const char* password  = "%yatengowifi%";

// Use WiFiClient class to create TCP connections
WiFiClient client;

// Variable Setup to manage the connections.
long    lastConnectionTime = 0;
int     failedCounter      = 0;
boolean lastConnected      = false;
int     uploadCounter      = 1;
int     blinkCounter       = 1;
const int httpPort         = 80;

// Time interval in milliseconds to update ThingSpeak (number of seconds * 1000 = interval)
const int IntervaloSubidaDatos_C = 60 * 1000;

// BMP180: Variable declaration for the BMP180 barometer.
SFE_BMP180 pressure;
typedef struct BMP180_type
{
   char    STATUS      = 0; // Whether the sensor give data or not.
   double  P_mBa       = 0; // Read pressure in mBar.
   double  HIGH_m      = 0; // Elevation in m.
   double  TEMPERATURE = 0; // Read Temperature ºC.
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
   double        particleMass;
};
DSM501A_type DSM501A_data;

// MiCS-6814: Variable declaration for the MiCS-6814 Multichanel Gas Sensor.
typedef struct MiCS_6814_type
{
   float NH3;    // NH3 concentration (ppm).
   float CO;     // CO concentration (ppm).
   float NO2;    // NO2 concentration (ppm).
   float C3H8;   // C3H8 concentration (ppm).
   float C4H10;  // C4H10 concentration (ppm).
   float CH4;    // CH4 concentration (ppm).
   float H2;     // H2 concentration (ppm).
   float C2H5OH; // C2H5OH concentration (ppm).
};
MiCS_6814_type MiCS_6814_data;

//Function prototypes, declaration of functions used in following code
void Start_BMP180_Sensor();
void Read_Dust_Sensor();
void Read_Barometer( );
void Read_Multichanel_Gas_Sensor();
void updateCarriots();
void updatePushingbox();
void updateThingSpeak();
void updateThingSpeak_1();
void updateemoncms();

void setup() {
   Serial.begin(115200);
   delay(10);

   // Initialize the sensor BMP180 => No funciona con este micro.
   // Start_BMP180_Sensor();
   // Declare LED pin for DSM501A;
   pinMode(D8, INPUT);
   pinMode(BUILTIN_LED, OUTPUT); // Onboard LED

   // Declare pin for DSM501A particle sensor;
   DSM501A_data.START_TIME = millis();

   // We start by connecting to a WiFi network
   // Se comienza conectando a la WiFi
   Serial.println();

   Serial.println("*********************************");
   Serial.println("* Iniciando WEMOS.");
   Serial.println("*");
   Serial.print("* WiFi connecting to ");
   Serial.print(WIFI_ssid);

   WiFi.begin(WIFI_ssid, password);

   // Repeat until the conexion is established
   while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");  
   }

   //The Wifi connection has been opened.
   Serial.println(" connected.");
   Serial.print("* IP address: ");
   Serial.println(WiFi.localIP());
   
   // Inicializacion de la libreria de Thingspeak
   ThingSpeak.begin(client);
   
   //Initialize the sensor BMP180
   Serial.println("*");
   Serial.print("*   Barometer   : ");
   Start_BMP180_Sensor();

   //Initialize the sensor BMP180
   Serial.println("*   Dust Sensor : DSM501A initialised.");

   // Initialise Multichanel Gas Sensor
   Serial.print("*   Gas Sensor  : ");
   gas.begin(0x04);//the default I2C address of the slave is 0x04
   gas.powerOn();

   Serial.println("* ");
   Serial.println("* Inicializacion correcta del micro.");
   Serial.println("*********************************");
}

void Start_BMP180_Sensor() {

   // Sensor initialization sequence.
   if (pressure.begin())
   {
      //Serial.println("* BMP180 init success");
      Serial.println("BMP180 initialised.");
   }
   else
   {

      Serial.println("BMP180 init fail (disconnected?)\n\n");
      while (1);
   }

   // Temperature initialization.
   BMP180_data.STATUS = pressure.startTemperature();
   if (BMP180_data.STATUS != 0)
   {
      delay(BMP180_data.STATUS);
      //Initial Temperature.
      BMP180_data.STATUS = pressure.getTemperature(BMP180_data.TEMPERATURE);
      if (BMP180_data.STATUS != 0)
      {
        // Pressure initialization.
         BMP180_data.STATUS = pressure.startPressure(3);
         if (BMP180_data.STATUS != 0)
         {
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
}

// Subprogram to read the DSM501A dust sensor: Concentration and particle mass.
void Read_Dust_Sensor() {
      DSM501A_data.DURATION = pulseIn(D8, LOW);  //Measures the time of a single low pulse
      DSM501A_data.LOW_PULSE_OCCUPANCY += DSM501A_data.DURATION;  //Measures total time of low pulse
      DSM501A_data.END_TIME = millis();

   if (( DSM501A_data.END_TIME - DSM501A_data.START_TIME) > DSM501A_data.SAMPLE_TIME)
   {

      DSM501A_data.RATIO = DSM501A_data.LOW_PULSE_OCCUPANCY
               /
             ( DSM501A_data.SAMPLE_TIME*10.0 );  //Converts to ratio of low time
       
      DSM501A_data.CONCENTRATION =
         1.1*pow(DSM501A_data.RATIO,3)-
         3.8*pow(DSM501A_data.RATIO,2)+
         520*DSM501A_data.RATIO+0.62; // using spec sheet curve
      //concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // Calculates concentration using spec sheet curve
      DSM501A_data.CONCENTRATION = DSM501A_data.CONCENTRATION * (3531.46); //Converts to Cubic meter
      Serial.print(millis());
      Serial.print(" DSM501A  : Concentration = ");
      Serial.print(DSM501A_data.CONCENTRATION);
      Serial.print(" pcs/m3");

      DSM501A_data.particleMass = DSM501A_data.RATIO * 0.11667; //Calculated as linear interpolation of spec data, as 1.14/0.12. (table 8.2)
      Serial.print("  Particle mass = ");
      Serial.print(DSM501A_data.particleMass, 4);
      Serial.print(" mg/m3");
//      Serial.println("\t\t\t*");
    
    Serial.print("  Ratio = ");
//    Serial.print("*Ratio = ");
    Serial.println(DSM501A_data.RATIO,4);
//    Serial.println("\t\t\t\t*");
    
//    Serial.println("*****************************************");
      DSM501A_data.LOW_PULSE_OCCUPANCY = 0;
      DSM501A_data.START_TIME = millis();
   }
}

// Subprogram to read the BMP085 barometer: Pressure, Temperature and Elevation
void Read_Barometer( ){

   if (BMP180_data.STATUS == 0)
   {
      //Barometer sensor status is not available. Initialize the sensor BMP180
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

               // The read of the Pressure is affected by the temperature
               // affecting to the sensor readings).
               BMP180_data.STATUS =
                  pressure.getPressure
                     (BMP180_data.P_mBa, BMP180_data.TEMPERATURE);
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
   // Write data
   Serial.print(millis());
   Serial.print(" BMP180   : T: ");
   Serial.print(BMP180_data.TEMPERATURE, 2); //display 2 decimal places
   Serial.print(" ");
   Serial.print((char)176);
   Serial.print("C  P: ");
   Serial.print(BMP180_data.P_mBa, 0); //whole number only.
   Serial.print(" mBar ");
   Serial.print(" Altitude: ");
   Serial.print(BMP180_data.HIGH_m, 2); //display 2 decimal places
   Serial.print(" m");
   Serial.println();
  
}

// Subprogram to read the MiCS-6814 Mutichannel Gas Sensor: 
//  NH3, CO, NO2, C3H8, C4H10, CH4, H2 and C2H5OH
void Read_Multichanel_Gas_Sensor()
{
    Serial.print(millis());
    Serial.print(" Gas (ppm): ");

    MiCS_6814_data.NH3 = gas.measure_NH3();
    Serial.print("NH3: ");
    if(MiCS_6814_data.NH3>=0) Serial.print(MiCS_6814_data.NH3);
    else Serial.print("invalid");

    MiCS_6814_data.CO = gas.measure_CO();
    Serial.print(" CO: ");
    if(MiCS_6814_data.CO>=0) Serial.print(MiCS_6814_data.CO);
    else Serial.print("invalid");

    MiCS_6814_data.NO2 = gas.measure_NO2();
    Serial.print(" NO2: ");
    if(MiCS_6814_data.NO2>=0) Serial.print(MiCS_6814_data.NO2);
    else Serial.print("invalid");

    MiCS_6814_data.C3H8 = gas.measure_C3H8();
    Serial.print(" C3H8: ");
    if(MiCS_6814_data.C3H8>=0) Serial.print(MiCS_6814_data.C3H8);
    else Serial.print("invalid");

    MiCS_6814_data.C3H8 = gas.measure_C4H10();
    Serial.print(" C4H10: ");
    if(MiCS_6814_data.C3H8>=0) Serial.print(MiCS_6814_data.C3H8);
    else Serial.print("invalid");

    MiCS_6814_data.CH4 = gas.measure_CH4();
    Serial.print(" CH4: ");
    if(MiCS_6814_data.CH4>=0) Serial.print(MiCS_6814_data.CH4);
    else Serial.print("invalid");

    MiCS_6814_data.H2 = gas.measure_H2();
    Serial.print(" H2: ");
    if(MiCS_6814_data.H2>=0) Serial.print(MiCS_6814_data.H2);
    else Serial.print("invalid");

    MiCS_6814_data.C2H5OH = gas.measure_C2H5OH();
    Serial.print(" C2H5OH: ");
    if(MiCS_6814_data.C2H5OH>=0) Serial.print(MiCS_6814_data.C2H5OH);
    else Serial.print("invalid");

    delay(1000);
    Serial.println("");
}

// Send stream to Carriots
void updateCarriots()
{
   // Variable declaration to work with Carriots

   const String Carriots_APIKEY = "5917b4cb109c087d1707f6179cc5593f6656c663f8a897469601632488c46611"; // Carriots apikey
   const String DEVICE = "ProyectoArduino@pinfocal.pinfocal"; // Replace with the id_developer of your device
   IPAddress carriotsAddress(82,223,244,60);  // api.carriots.com IP Address

   Serial.print(millis());
   Serial.print(" Carriots: ");
   if (client.connect(carriotsAddress, httpPort))
   {  // Si la conexi?n se ha establecido.
      // Genera el campo de datos.
      String json_url = 
         "{\"protocol\":\"v2\",\"device\":\""+DEVICE+
          "\",\"at\":\"now\",\"data\":{\"Temperatura\":\""+BMP180_data.TEMPERATURE+
          "\",\"Presion\":\""+BMP180_data.P_mBa+
          "\",\"Concentracion\":\""+DSM501A_data.CONCENTRATION+
          "\",\"Ratio\":\""+DSM501A_data.RATIO+
          "\",\"Masa\":\""+DSM501A_data.particleMass+
          "\",\"Monoxido\":\""+MiCS_6814_data.CO+
          "\",\"Dioxido\":\""+MiCS_6814_data.NO2+
          "\",\"Metano\":\""+MiCS_6814_data.CH4+"\"}}";              
              
      // Genera una petici?n HTTP
      client.println("POST /streams HTTP/1.1");
      client.println("Host: api.carriots.com");
      client.println("Accept: application/json");
      client.println("User-Agent: Arduino-Carriots");
      client.println("Content-Type: application/json");
      client.print("carriots.apikey: ");
      client.println(Carriots_APIKEY);
      client.print("Content-Length: ");
      client.println(json_url.length());
      client.println(json_url);
      client.println("Connection: close");
      client.println();
      client.println(json_url);

      delay(1000);
      lastConnectionTime = millis();
      delay(1000);
      if (client.connected())
      {
         //Serial.println("Connecting to Carriots...");
         Serial.print(" Subiendo Datos.");
         failedCounter = 0;
      }
      else
      {
         failedCounter++;
         Serial.println("Error_1: Connection to Carriots failed (" +
                         String(failedCounter, DEC) + ")");
         Serial.println();
      }
      delay(1000);

   }
   else
   {
      failedCounter++;
      Serial.println("Error_2: Connection to Carriots Failed (" +
                      String(failedCounter, DEC) + ")");
      Serial.println();
      lastConnectionTime = millis();
   }
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
      Serial.print(".");
      //Serial.print(line);
   }

   Serial.println(" OK.");
} // end updateCarriots

void updatePushingbox()
{
   Serial.print(millis());
   char pushingbox_Address[] = "api.pushingbox.com";
   // THIS IS THE DEVICE ID FROM PUSHINGBOX
   char pushingbox_ID[] = "vC6A8513890F3CEF";
   //char pushingbox_ID[] = "vEBD38E81685020D";
   
   char pushingbox_msg[100];

   //Connect to wifi
   Serial.print(" Pushingbox: ");

   if (client.connect(pushingbox_Address, httpPort))
   {
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
      //Serial.println("........1..");

      data="";
      data+="";
      
      data+="&Temperatura=";
      static char CHAR_TEMPERATURE[10];
      dtostrf(BMP180_data.TEMPERATURE,5, 2, CHAR_TEMPERATURE); // Pasa el double a char.
      data+=CHAR_TEMPERATURE;
      
      data+="&Presion=";
      static char CHAR_P_mBa[10];
      dtostrf(BMP180_data.P_mBa,5, 2, CHAR_P_mBa); // Pasa el double a char.
      data+=CHAR_P_mBa;
      
      data+="&Concentracion=";
      static char CHAR_CONCENTRATION[10];
      dtostrf(DSM501A_data.CONCENTRATION,9, 2, CHAR_CONCENTRATION); // Pasa el double a char.
      data+=CHAR_CONCENTRATION;

      data+="&Ratio=";
      static char CHAR_RATIO[10];
      dtostrf(DSM501A_data.RATIO,5, 2, CHAR_RATIO); // Pasa el double a char.
      data+=CHAR_RATIO;

      data+="&Masa=";
      static char CHAR_particleMass[10];
      dtostrf(DSM501A_data.particleMass,4, 2, CHAR_particleMass); // Pasa el double a char.
      data+=CHAR_particleMass;

      data+="&CO=";
      static char CHAR_CO[10];
      dtostrf(MiCS_6814_data.CO,5, 2, CHAR_CO); // Pasa el double a char.
      data+=CHAR_CO;

      data+="&NO2=";
      static char CHAR_NO2[10];
      dtostrf(MiCS_6814_data.NO2,4, 2, CHAR_NO2); // Pasa el double a char.
      data+=CHAR_NO2;

      data+="&CH4=";
      static char CHAR_CH4[10];
      dtostrf(MiCS_6814_data.CH4,11, 2, CHAR_CH4); // Pasa el double a char.
      data+=CHAR_CH4;
 /*     data+="&Todo=";
      data+=" ";
      data+="&entry.2131645315=";
      data+=CHAR_TEMPERATURE;
 */
     data=data+"&Todo="+" "+"&entry.2131645315="+CHAR_TEMPERATURE+"&entry.468221150="+CHAR_P_mBa;

      data+="&status=25";
      data+= "&&submit=Submit";

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

      delay(1000);
      lastConnectionTime = millis();
      delay(1000);
      if (client.connected())
      {
         Serial.print(" Subiendo Datos.");
         failedCounter = 0;
      }
      else
      {
         failedCounter++;
         Serial.println("Error_3: Connection to Pushingbox failed (" +
                         String(failedCounter, DEC) + ")");
         Serial.println();
      }
      delay(1000);
   }
   else
   {
      failedCounter++;
      Serial.println("Error_4: Connection to Pushingbox Failed (" +
                      String(failedCounter, DEC) + ")");
      Serial.println();
      lastConnectionTime = millis();
   }
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
      Serial.print(".");
      //Serial.print(line);
   }

   Serial.println(" OK.");
 } //end updatePushingbox;


void updateThingSpeak()
{
   // Variable declaration to work with thingspeak
   // ThingSpeak Settings
   char thingSpeakAddress[] = "api.thingspeak.com";
   String writeAPIKey_C = "7J5F3NW8FDLOJDX8";
   //String writeAPIKey_C = "vEBD38E81685020D";
      
/*    // Google forms Settings
   char Google_form_Address[] = "www.google.es";
   char Google_form_key[] = "1cRJwf9MiV4jKkmAiYpPgx1Uzce4K-DZQ2v0l0ywWUZY"; //Replace with your Key
 */   //https://docs.google.com/forms/d/1cRJwf9MiV4jKkmAiYpPgx1Uzce4K-DZQ2v0l0ywWUZY/e
   Serial.print(millis());
   Serial.print(" ThingSpeak: ");

   if (client.connect(thingSpeakAddress, httpPort))
   {
      // Build the data field
      String tsData = "field1="+String(BMP180_data.TEMPERATURE,DEC)
                    +"&field2="+String(BMP180_data.P_mBa,DEC)
                    +"field3="+String(DSM501A_data.CONCENTRATION,DEC)
                    +"field4="+String(DSM501A_data.RATIO,DEC)
                    +"field5="+String(DSM501A_data.particleMass,DEC)
                    +"field6="+String(MiCS_6814_data.CO,DEC)
                    +"field7="+String(MiCS_6814_data.NO2,DEC)
                    +"field8="+String(MiCS_6814_data.CH4,DEC);
      String tsData_1 = "field1="+String(BMP180_data.TEMPERATURE,DEC)
                    +"&field2="+String(BMP180_data.P_mBa,DEC);
      String tsData_2 = "field3="+String(DSM501A_data.CONCENTRATION,DEC)
                    +"field4="+String(DSM501A_data.RATIO,DEC);
      String tsData_3 = "field5="+String(DSM501A_data.particleMass,DEC)
                    +"field6="+String(MiCS_6814_data.CO,DEC);
      String tsData_4 = "field7="+String(MiCS_6814_data.NO2,DEC)
                    +"field8="+String(MiCS_6814_data.CH4,DEC);

          // Make a HTTP request
      client.print("POST /update HTTP/1.1\n");
      client.print("Host: api.thingspeak.com\n");
      client.print("Connection: close\n");
      client.print("X-THINGSPEAKAPIKEY: " + writeAPIKey_C + "\n");
      client.print("Content-Type: application/x-www-form-urlencoded\n");
      client.print("Content-Length: ");
//      client.println(tsData.length());
      client.println(tsData_3.length());
      client.println();
      client.print(tsData);
      client.print(tsData_3);
//
//Serial.print(tsData);
//
      delay(1000);
      lastConnectionTime = millis();
      delay(1000);
      if (client.connected())
      {
         Serial.print(" Subiendo Datos.");
         failedCounter = 0;
      }
      else
      {
         failedCounter++;
         Serial.println("Error_5: Connection to ThingSpeak failed (" +
                         String(failedCounter, DEC) + ")");
         Serial.println();
      }
      delay(1000);
   }
   else
   {
      failedCounter++;
      Serial.println("Error_6: Connection to ThingSpeak Failed (" +
                      String(failedCounter, DEC) + ")");
      Serial.println();
      lastConnectionTime = millis();
   }
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
      Serial.print(".");
      Serial.print(line);
      //Serial.print(line);
   }

   Serial.println(" OK.");
}  //updateThingSpeak


void updateThingSpeak_1()
{
   // Declaraci?n de variables para trabajar con thingspeak
   // Configuraci?n para ThingSpeak
   char thingSpeakAddress[] = "api.thingspeak.com";
   unsigned long thingspeak_channel_number_C = 101717;
   const char * writeAPIKey_C = "7J5F3NW8FDLOJDX8";
   // Se pasan las variables a float.
   float TEMPERATURE_LOG = (float) BMP180_data.TEMPERATURE;
   float PRESURE_LOG     = (float) BMP180_data.P_mBa;
   float MASS_LOG        = (float) DSM501A_data.particleMass;

   // Actualiza los campos.
   ThingSpeak.setField(1,TEMPERATURE_LOG);
   ThingSpeak.setField(2,PRESURE_LOG);
   ThingSpeak.setField(3,DSM501A_data.CONCENTRATION);
   ThingSpeak.setField(4,DSM501A_data.RATIO);
   ThingSpeak.setField(5,MASS_LOG);
   ThingSpeak.setField(6,MiCS_6814_data.NO2);
   ThingSpeak.setField(7,MiCS_6814_data.CO);
   ThingSpeak.setField(8,MiCS_6814_data.CH4);
  
   // Escribe los campos que se han actualizado.
   ThingSpeak.writeFields(thingspeak_channel_number_C, writeAPIKey_C);  
    
  }  //updateThingSpeak_1


void updateemoncms()

{  
   char emoncms_Address[] = "emoncms.org";
   // THIS IS THE DEVICE ID FROM PUSHINGBOX
   char emoncms_APIKEY[] = "cca68e0f95bf2fbb3cd12ba92ae64b38";
   //Connect to wifi
   //Unique node, kind of unique sensor number
   int node;

   Serial.print(millis());
   Serial.print(" emoncms: ");
   if (client.connect("emoncms.org", httpPort)) 
   {
      //Serial.print("Connected to server");
      // Si la conexi?n se ha realizado.
      // Genera el formulario JSON para los datos del Bar?metro.
      node = 1;
      String json_url_1 = " /input/post.json?node=" + String(node) + 
              "&json={Temperatura:" + String(BMP180_data.TEMPERATURE) +  
                        ",Presion:" + String(BMP180_data.P_mBa) +  
              "}&apikey=" + emoncms_APIKEY; 
      // Genera el formulario JSON para los datos del Sensor de Part?culas.
      node = 2;
      String json_url_2 = " /input/post.json?node=" + String(node) + 
              "&json={Concentration:" + String(DSM501A_data.CONCENTRATION) +  
              "}&apikey=" + emoncms_APIKEY; 
      String json_url_3 = " /input/post.json?node=" + String(node) + 
              "&json={Ratio:" + String(DSM501A_data.RATIO) +  
                             ",Masa:" + String(DSM501A_data.particleMass) +  
              "}&apikey=" + emoncms_APIKEY; 
      node = 3;
      // Genera el formulario JSON para los datos del Multichanel Gas Sensor.
      String json_url_4 = " /input/post.json?node=" + String(node) + 
              "&json={CO:" + String(MiCS_6814_data.CO) +  
                   ",NO2:" + String(MiCS_6814_data.NO2) +  
                   ",CH4:" + String(MiCS_6814_data.CH4) +  
              "}&apikey=" + emoncms_APIKEY; 
 
  
      String host = "Host: emoncms.org\n";

      // Cadena para mandar los datos del barometro al servidor
      String requestString = String("GET ") + json_url_1  + " HTTP/1.1\n"+ host + "Connection: close\r\n\r\n";
     // Serial.print("Request String: " + requestString);
      client.print(requestString ); // Manda la peticion HTTP al servidor.
  //Serial.println("********* Response 1 *********");
  //Wait for server to respond and print '-'
  while(!client.available()) {
    Serial.print(" ");
    delay(200); 
  }

  //Write out what the server responds with
  while(client.available()){
//    Serial.write(client.read());
      String line = client.readStringUntil('\r');
//      Serial.print(".");
  }

  if (!client.connect("emoncms.org", httpPort)) {
    Serial.println("connection failed");
    return;
  }

      delay(1000);
      // Cadena para mandar los datos del sensor de part?culas al servidor
      requestString = String("GET ") + json_url_2  + " HTTP/1.1\n"+ host + "Connection: close\r\n\r\n";
//      Serial.print("Request String: " + requestString);
      client.print(requestString );

  //Serial.println("********* Response 2 *********");
  //Wait for server to respond and print '-'
  while(!client.available()) {
    Serial.print(" ");
    delay(200); 
  }

  //Write out what the server responds with
  while(client.available()){
//    Serial.write(client.read());
      String line = client.readStringUntil('\r');
      Serial.print(".");
  }

  if (!client.connect("emoncms.org", httpPort)) {
    Serial.println("connection failed");
    return;
  }
      delay(1000);
      // Cadena para mandar los datos del multichanel gas sensor al servidor
      requestString = String("GET ") + json_url_3  + " HTTP/1.1\n"+ host + "Connection: close\r\n\r\n";
//      Serial.print("Request String: " + requestString);
      client.print(requestString );
  //Serial.println("********* Response 3 *********");
  //Wait for server to respond and print '-'
  while(!client.available()) {
    Serial.print(" ");
    delay(200); 
  }

  //Write out what the server responds with
  while(client.available()){
//    Serial.write(client.read());
      String line = client.readStringUntil('\r');
      Serial.print(".");
  }

  if (!client.connect("emoncms.org", httpPort)) {
    Serial.println("connection failed");
    return;
  }

      delay(1000);
      // This will send the request to the server
      requestString = String("GET ") + json_url_4  + " HTTP/1.1\n"+ host + "Connection: close\r\n\r\n";
//      Serial.print("Request String: " + requestString);
      client.print(requestString );

      delay(1000);
      lastConnectionTime = millis();
      delay(1000);
      if (client.connected())
      {
         Serial.print(" Subiendo Datos.");
         failedCounter = 0;
      }
      else
      {
         failedCounter++;
         Serial.println("Error_7: Connection to emoncms failed (" +
                         String(failedCounter, DEC) + ")");
         Serial.println();
      }
      delay(1000);
   }
   else
   {
      failedCounter++;
      Serial.println("Error_8: Connection to emoncms Failed (" +
                      String(failedCounter, DEC) + ")");
      Serial.println();
      lastConnectionTime = millis();
   }
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
      Serial.print(".");
      //Serial.print(line);
   }

   Serial.println(" OK.");
 } //end updateemoncms;

void loop() {

  // Sensors are only read when the arduino is not uploading data to the network.
  if (!client.connected())
  {

      //Read the BMP180 barometer
      Read_Barometer();
      delay(5000);

      //Read the DSM501A dust sensor
      Read_Dust_Sensor();
      delay(5000);

    // Read the MiCS-6814 Mutichannel Gas Sensor: 
      //  NH3, CO, NO2, C3H8, C4H10, CH4, H2 and C2H5OH
      Read_Multichanel_Gas_Sensor();
      delay(5000);
  }
  else
  {
     Serial.println(" client connected");
  }
   // Update data to the IoT server
   if(!client.connected() && (millis() - lastConnectionTime > IntervaloSubidaDatos_C))
   {
      //Set the number of blinks counter.
      blinkCounter = uploadCounter;

      //Serial.print(" uploadCounter: ");
      //Serial.println(uploadCounter);

      switch (uploadCounter)
      {
         case 1:
            // Upload the data to Carriots.
            updateCarriots();
//            updateThingSpeak_1();
            // Reset the variable.
            uploadCounter = 2;
            break;

         case 2:
            // Upload the Monitor data.
//            updatePushingbox();
            updateCarriots();
//            updateThingSpeak_1();
            // Change variable value.
            uploadCounter = 3;
            break;

         case 3:
            // Upload the Monitor data.
            updateCarriots();
//            updateThingSpeak_1();
            // Change variable value.
            uploadCounter = 4;
            break;
         case 4:
            // Upload the Monitor data.
            updateemoncms();
//            updateCarriots();
//            updateThingSpeak_1();
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
} //end loop
