https://productforums.google.com/forum/plain/msg/docs/f4hJKF1OQOw/hzLT2Fz6ZSwJ

#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <SPI.h>
#include <Ethernet.h> 
#include "DHT.h"
Adafruit_BMP085 bmp;

#define DHTTYPE DHT22   			// DHT 22  (AM2302)
#define DHT_1PIN 5  
#define DHT_2PIN 6  
#define DHT_3PIN 7     			// what pin we're connected to
#define DHT_4PIN 8 
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
			// buffer string for float to string conversion

unsigned long pollingWait = 600000; 		//10 minute wait before polling again
DHT dht_1(DHT_1PIN, DHTTYPE);
DHT dht_2(DHT_2PIN, DHTTYPE);
DHT dht_3(DHT_3PIN, DHTTYPE);
DHT dht_4(DHT_3PIN, DHTTYPE);
byte mac[] = {0x90, 0xA2, 0xDA, 0x0E, 0xFB, 0x20}; 	//Correct MAC Address 
boolean DEBUG = false; 				// Debug mode
EthernetClient client;
char server [] = "api.pushingbox.com";
String API= "v3652C8EA7BFC0EA";
 
unsigned long lastConnectionTime = 0; 		// last time you connected to the server, in milliseconds
boolean lastConnected  = false; 			// state of the connection last time through the main loop
const unsigned long postingInterval = pollingWait; 	// delay between updates, in milliseconds
float InTemp=0;
  float InHum=0;
  float AtticTemp=0;
  float AtticHum=0;
  float BMPTemp =99;
  float BarometerPress=20;
  float CrawlTemp=0;
  float CrawlHum=0;
  float OutTemp=0;
  float OutHum=0;
  
void setup () {
  Serial.begin (9600);
  Serial.println("Initialising postingInterval at ");
  Serial.println(postingInterval);

  bmp.begin(); //Start BMP sensor
  dht_1.begin(); //Start DHT sensors
  dht_2.begin();
  dht_3.begin();
  dht_4.begin();
  Ethernet.begin(mac);
  
  Serial.println("Ethernet ready");
  Serial.print("My IP address: ");    
  Serial.println(Ethernet.localIP());
  
  delay(1000);
  }
 
void loop () {
  Serial.println("Start Sensor data gather");
  AtticTemp = (dht_1.readTemperature()*1.8+32);
  AtticHum = dht_1.readHumidity();
  InTemp = (dht_2.readTemperature()*1.8+32);
  InHum = dht_2.readHumidity();
  BMPTemp = (bmp.readTemperature()*1.8+32);
  BarometerPress = (bmp.readPressure()*0.000295299830714);
  CrawlTemp = (dht_3.readTemperature()*1.8+32);
  CrawlHum = dht_3.readHumidity();
  OutTemp = (dht_4.readTemperature()*1.8+32);
  OutHum = dht_4.readHumidity();
  delay(1000);
  
  Serial.println("data gathered!!!!!");
   
  // If there's no net connection, but there WAS one last time
  // Through the loop, THEN stop the client:
  if (!client.connected() && lastConnected) {
    Serial.println ();
    Serial.println ("disconnecting.");
    client.stop ();
  }
 
  // If you're not connected, and ten seconds have Passed since
  // Your last connection, then connect again and send date:
  Serial.print ("Client Connected: ");
  Serial.println (client.connected());
  Serial.print ("milis - lastConnectionTime > postingInterval? : ");
  Serial.println ((millis() - lastConnectionTime > postingInterval));
  if (!client.connected() && (millis() - lastConnectionTime > postingInterval)) {
    Serial.println ("About to send..");
    httpRequest ();
  }
  // Store the state of the connection for next time through
  // The loop:
  lastConnected = client.connected();

}
 
// This method Makes a HTTP connection to the server:
void httpRequest () {
 
  data="";
  data+="";
  
  data+="&AtticTemp=";
  dtostrf(AtticTemp,4,2,str1);
  data+=str1;     
  data+="&AtticHum=";
  dtostrf(AtticHum,4,2,str2);
  data+=str2;
  data+="&InTemp=";
  dtostrf(InTemp,4,2,str3);
  data+=str3; 
  data+="&InHum=";
  dtostrf(InHum,4,2,str4);
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
  data+= "&&submit=Submit";
  
  
  Serial.println ();
  Serial.println ("The http data string is ...");
  Serial.println(data);
  // If there's a successful connection:
  if (client.connect(server, 80)) {
    Serial.println ("connecting ...");
    
    // Send the HTTP PUT request:
    client.print("POST /pushingbox/pushingbox?devid=");
    client.print(API);
    client.println(" HTTP/1.1");
    client.println("Host: api.pushingbox.com");
    client.println("Content-Type: application/x-www-form-urlencoded");
   
    client.println("Connection: close");
    client.print("Content-Length: ");
    client.println(data.length());
    client.println();
    client.print(data);
    client.println();
    delay(100);
    client.stop(); 
 
    // Note the time That the connection was made:
    lastConnectionTime = millis();
  } 
  else {
    // If you couldn't make the connection:
    Serial.println("connection failed");
    Serial.println("disconnecting.");
    client.stop();
  }
}