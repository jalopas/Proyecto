I do hope that you have figured it out by now! But I have finally gotten the information off of my dead computer, which included my code and found the time to sit down and pull this together again. Below is my code. I have not looked at it since the end of January so my head is not in it right now. It has a lot of stuff in there. When I left off I was trying to get it to keep track of the time the fan on my furnace was on. I can get it to post to Google docs when it is on and when it is off so I can figure how long it is on but when I try to have the arduino keep track of the time it is on the program crashes. Right now though I believe that this code is stable. But don't quote me on that ;) I am by no means an expert here. This code is a patch work of what I have found from other places. so I can't say I understand it particulary well but what I think I am doing here is converting the numbers that I get from my temperature sensors to a string. I don't see you doing that in your code. Don't know if that will make the difference. If you haven't figured it out yet then hopefully my code will help you. If you have questions then let me know now that I have my computer back up and running I should be able to be more responsive. 
https://productforums.google.com/forum/#!msg/docs/f4hJKF1OQOw/9k2oJ-kLULsJ

#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "EmonLib.h" 

byte mac[] = {0x90, 0xA2, 0xDA, 0x0D, 0x9C, 0xFD}; //Replace with your Ethernet shield MAC
byte ip[] = { 192,168,1,105}; //The Arduino device IP address
EthernetClient client1;
EthernetClient client2;
EthernetClient client3;
EnergyMonitor emon1; 
unsigned long epoch;

//time keeping
unsigned int localPort = 8888;      // local port to listen for UDP packets
IPAddress timeServer(132, 163, 4, 101); // time-a.timefreq.bldrdoc.gov NTP server
const int NTP_PACKET_SIZE= 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
EthernetUDP Udp; // A UDP instance to let us send and receive packets over UDP

char server[] = "api.pushingbox.com";

String API ="v52CD6A9CB77EACB";

// ThingSpeak Settings
char thingSpeakAddress[] = "api.thingspeak.com";
String writeAPIKey = "B9RABJAZWE3WZ38C";
String writeAPIKey2 = "0VJUKE9VHB77JJ5Z";

//variables for comparing temperatures
float comp1;
float comp2;
float comp3;
float comp4;
float comp5;
float math1;
float math2;
float math3;
float math4;
float math5;
float loss1;
//float emailprevtime=0;
//float timeon=0;
//float timeOninvt=0;
//float HDD=0;
//float HDDtime;
//int HDDsetpoint=68;

#define ONE_WIRE_BUS 3

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

DeviceAddress furnaceout = { 0x28, 0x16, 0xED, 0x74, 0x04, 0x00, 0x00, 0xF9 };
DeviceAddress southbed = { 0x28, 0xA0, 0xA8, 0x75, 0x04, 0x00, 0x00, 0xF8 };
DeviceAddress furnacein = { 0x28, 0x8D, 0xD7, 0x75, 0x04, 0x00, 0x00, 0x06 };
DeviceAddress hallin = { 0x28, 0x92, 0x96, 0x74, 0x04, 0x00, 0x00, 0xD2 };
DeviceAddress officeout = { 0x28, 0xD3, 0x6E, 0x75, 0x04, 0x00, 0x00, 0x87 };
DeviceAddress crawl = { 0x28, 0x70, 0xAA, 0x75, 0x04, 0x00, 0x00, 0xBF };
DeviceAddress south = { 0x28, 0xA3, 0xB5, 0x75, 0x04, 0x00, 0x00, 0x31 };
DeviceAddress out = { 0x28, 0x38, 0xA3, 0x05, 0x05, 0x00, 0x00, 0x9B };

void setup()
{
   Serial.begin(9600);
   //Ethernet.begin(mac, ip ,dnserv, gateway , subnet);
   Ethernet.begin(mac, ip);
   delay(1000);

   emon1.current(1, 111.1);             // Current: input pin, calibration.

   Serial.println( Ethernet.localIP()); // ------> just to check
   Serial.println( Ethernet.subnetMask()); // ------> just to check
   Serial.println( Ethernet.gatewayIP()); // ------> just to check
   Serial.println( Ethernet.dnsServerIP()); // ------> just to check

   Serial.println("connecting…");
   sensors.begin();
   //dht.begin();
   sensors.setResolution(furnaceout, 11);
   sensors.setResolution(southbed, 11);
   sensors.setResolution(furnacein, 11);
   sensors.setResolution(hallin, 11);
   sensors.setResolution(officeout, 11);
   sensors.setResolution(crawl, 11);
   sensors.setResolution(south, 11);
   sensors.setResolution(out, 11);
   
   //this was added to turn off the SD card reader. Should add stability to the sketch
   pinMode(4,OUTPUT);
   digitalWrite(4,HIGH);
   Udp.begin(localPort);
 }

void printTemperature(DeviceAddress deviceAddress)
{
  float tempC = sensors.getTempC(deviceAddress);
  if (tempC == -127.00) {
    Serial.print("Error getting temperature");
  } else {
    Serial.print("C: ");
    Serial.print(tempC);
    Serial.print(" F: ");
    Serial.print(DallasTemperature::toFahrenheit(tempC));
    }
}

void loop(){
  
  sendNTPpacket(timeServer); // send an NTP packet to a time server
     // wait to see if a reply is available
  delay(1000);  
  if ( Udp.parsePacket() ) {  
    // We've received a packet, read the data from it
    Udp.read(packetBuffer,NTP_PACKET_SIZE);  // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);  
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;  
    Serial.print("Seconds since Jan 1 1900 = " );
    Serial.println(secsSince1900);              

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;    
    // subtract seventy years:
    epoch = secsSince1900 - seventyYears;  
    // print Unix time:
    Serial.println(epoch);                              

    // print the hour, minute and second:
    Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
    Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
    Serial.print(':');  
    if ( ((epoch % 3600) / 60) < 10 ) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
    Serial.print(':');
    if ( (epoch % 60) < 10 ) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.println(epoch %60); // print the second
  }
  
float Irms = emon1.calcIrms(1480);  // Calculate Irms only

String data;
String data2;
String data3;
sensors.requestTemperatures();
float temp1 = sensors.getTempF(southbed);
float temp2 = sensors.getTempF(furnaceout);
float temp3 = sensors.getTempF(furnacein);
float temp4 = sensors.getTempF(officeout);
float temp5 = sensors.getTempF(hallin);
float temp6 = sensors.getTempF(crawl);
float temp7 = sensors.getTempF(furnaceout)-sensors.getTempF(south);
float temp8 = sensors.getTempF(out);
loss1=(sensors.getTempC(furnaceout)-sensors.getTempC(officeout))*0.000040912084099*100000;

char str1[6];
char str2[6];
char str3[6];
char str4[7];
char str5[6];
char str6[6];
char str7[6];
char str8[6];
char str9[6];
char str10[6];
char str11[7];

dtostrf(temp1,5, 2, str1);
dtostrf(temp2,5, 1, str2);
dtostrf(temp3,5, 2, str3);
dtostrf(temp4,6, 2, str4);
dtostrf(temp5,5, 2, str5);
dtostrf(loss1,5, 1, str6);
dtostrf(temp6,5, 2, str7);
dtostrf(temp7,5, 2, str8);
dtostrf(temp8,5, 2, str9);

data+="";
data+="";
data+="&field1=";  
data+=str1;
data+="&field2=";
data+=str2;
data+="&field3=";
data+=str3;
data+="&field4=";
data+=str4;
data+="&field5=";
data+=str5;
if(Irms*230>250 && Irms*230<1000){
  
Serial.print(Irms*230.0);           // Apparent power
Serial.print(" ");
Serial.println(Irms);               // Irms
Serial.println("the current sensor is above 250");
data+="&field6=";  
data+=str6;
//timeon=epoch - timeOninvt + timeon;
//timeOninvt=epoch;
}else{
//  timeOninvt=epoch;
  Serial.println("the current sensor is below 250");    
}
data+="&field7=";
data+=str7;
data+="&field8=";
data+=str8;

data2+="";
data2+="";
data2+="&field1=";  
data2+=str9;

 Serial.println(temp8);
  Serial.println(temp4);
  Serial.println(str6);
  //Serial.println(temp7);
  math1=temp1-comp1;
  math2=temp2-comp2;
  math3=temp3-comp3;
  math4=temp4-comp4;
  math5=temp8-comp5;
  


if (abs(math1)>5 || abs(math2)>5 || abs(math3)>5 || abs(math4)>5 || Irms*230>250 && Irms*230<1000){

if (client1.connect(server, 80)){

client1.print("POST /update HTTP/1.1\n");
    client1.print("Host: api.thingspeak.com\n");
    client1.print("Connection: close\n");
    client1.print("X-THINGSPEAKAPIKEY: "+writeAPIKey+"\n");
    client1.print("Content-Type: application/x-www-form-urlencoded\n");
    client1.print("Content-Length: ");
    client1.print(data.length());
    client1.print("\n\n");
client1.print(data);
client1.println();

 Serial.println();
 Serial.print(data);
 Serial.println();

 Serial.println("connected");
}
comp1=temp1;
comp2=temp2;
comp3=temp3;
comp4=temp4;
}

if (abs(math5)>1){
if (client2.connect(server, 80)){

client2.print("POST /update HTTP/1.1\n");
    client2.print("Host: api.thingspeak.com\n");
    client2.print("Connection: close\n");
    client2.print("X-THINGSPEAKAPIKEY: "+writeAPIKey2+"\n");
    client2.print("Content-Type: application/x-www-form-urlencoded\n");
    client2.print("Content-Length: ");
    client2.print(data2.length());
    client2.print("\n\n");
client2.print(data2);
client2.println();
}
comp5=temp8;

//HDD calcs
//HDD=(HDDsetpoint-temp8)*(epoch-HDDtime)/24/60/60+HDD;
//HDDtime=epoch;
}
//dtostrf(HDD,5, 2, str10);
//dtostrf(timeon/60,6, 2, str11);
data3+="";
data3+="";
data3+="&temp1=";  
//data3+=str11;
data3+="&HDDout=";  
//data3+=str10; 

//if (epoch - emailprevtime > 7200){
//  if (client3.connect(server, 80)){
//Serial.print("client 3 is connected");
//client3.print("POST /pushingbox/pushingbox?devid=");
//   client3.print(API);
//client3.println(" HTTP/1.1");
//client3.println("Host: api.pushingbox.com");
//client3.println("Content-Type: application/x-www-form-urlencoded");
//client3.println("Connection: close");
//client3.print("Content-Length: ");
//client3.println(data3.length());
//client3.println();
//client3.print(data3);
//client3.println();
//  }
//emailprevtime = epoch;
//timeon=0;
//HDD=0;
//  }
delay(1000);
client1.stop();
client2.stop();
//client3.stop();
delay(10000);
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:         
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer,NTP_PACKET_SIZE);
  Udp.endPacket();
}
