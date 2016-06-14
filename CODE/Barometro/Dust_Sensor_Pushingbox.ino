/* 
*  WiFi dust sensor with Arduino, the Sharp GP2Y1010AU0F, the CC3000 chip, PushingBox, & GoogleDocs
*  Written by Carl Lee McKinney
*/

// Include required libraries
#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>

// Define CC3000 interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   3
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10

// Define WiFi network
#define WLAN_SSID       "xxx"
#define WLAN_PASS       "xxx"
#define WLAN_SECURITY   WLAN_SEC_WPA2

// Create CC3000 instance
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, 
ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT, SPI_CLOCK_DIV2);

Adafruit_CC3000_Client client;

// Define variables for server and dev id
char serverName[] = "api.pushingbox.com";
char devId[] = "xxx";

// Set GP2Y1010AU0F sensor pins
int measurePin = 0;
int ledPower = 2;

// Define variables used for GP2Y1010AU0F sampling
int samplingTime = 280;
int deltaTime = 40;
int sleepTime = 9680;
int voMeasured = 0;

// Temporary fixed location data
char location[] = "xxx";
                             
void setup()
{ 
  // Set pin for LED power to GP2Y1010AU0F to OUTPUT
  pinMode (ledPower, OUTPUT);
  
  // Start serial
  Serial.begin(115200);

  // Initialize the CC3000
  if (!cc3000.begin())
  {
    Serial.println("Couldn't initialize CC3000");
    while(1);
  }

  // Connect to  WiFi network
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY))
  {
    Serial.println("Couldn't make WiFi connection");
    while(1);
  }
  
  Serial.println("Connected to WiFi!");
    
  // Check DHCP
  while (!cc3000.checkDHCP())
  {
    delay(100);
  }

}

void loop()
{
  for (int i=1; i<=5; i++){
  // Measure from GP2Y1010AU0F
  digitalWrite(ledPower,LOW); // power on the LED
  delayMicroseconds(samplingTime);
  
  voMeasured = analogRead(measurePin); // read the dust density value
  
  delayMicroseconds(deltaTime);
  digitalWrite(ledPower,HIGH); // turn the LED off
  delayMicroseconds(sleepTime);
  
  // Push to Google Docs via Pushing Box
  client.stop();
  if(client.connect(serverName, 80)) { 
    Serial.println("Connected to Pushingbox!");
    Serial.print("Location: ");
    Serial.print(location);
    Serial.print(", Dust Density: ");
    Serial.println(voMeasured);
    Serial.println("Sending request...");
    client.print("GET /pushingbox?devid=");
    client.print(devId);
    client.print("&location=");
    client.print(location);
    client.print("&dustDensity=");
    client.print(voMeasured);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(serverName);
    client.println("User-Agent: Arduino");
    client.println();
    client.println();
    
      // Debug
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }
  
    Serial.println("Disconnecting from Pushing Box...");
  } 
  else { 
    Serial.println("Connection to Pushingbox failed!"); 
  }
  delay(60000);
  }
  Serial.println("Disconnecting Wifi...");
  cc3000.disconnect();
}
