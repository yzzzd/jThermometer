#include "DHT.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define DHTTYPE DHT22
#define DHTPIN 4

DHT dht(DHTPIN, DHTTYPE);
String stringOne, stringTwo, stringThree, stringFour, data, dataIn;

WiFiUDP Udp;
unsigned int localUdpPort = 4210;
char incomingPacket[255];
char replyPacket[255];

WiFiServer server(4210);

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(2000);

  stringOne = String("{\"temperature celcius\":");
  stringTwo = String(",\"temperature fahrenheit\":");
  stringThree = String(",\"humidity\":");
  stringFour = String("}");

  WiFi.mode(WIFI_AP);
  WiFi.softAP("jThermometer", "", 1, false, 8);
  server.begin();

  Udp.begin(localUdpPort);

  while(!Serial) { }
}

int timeSinceLastRead = 0;
void loop() {
  if(timeSinceLastRead > 2000) {
    
    float hum = dht.readHumidity();
    float temp = dht.readTemperature();
    float fah = dht.readTemperature(true);

    if (isnan(hum) || isnan(temp) || isnan(fah)) {
      timeSinceLastRead = 0;
      return;
    }

    data = stringOne + temp + stringTwo + fah + stringThree + hum + stringFour;
    data.toCharArray(replyPacket, 255);
    
    timeSinceLastRead = 0;
  }

  int packetSize = Udp.parsePacket();
  if (packetSize) {
    int len = Udp.read(incomingPacket, 255);
    if (len > 0) {
      incomingPacket[len] = 0;
    }
    dataIn = String(incomingPacket);

    if(dataIn.indexOf("get_data") != -1) {
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      Udp.write(replyPacket);
      Udp.endPacket();
    }
  }
  
  delay(100);
  timeSinceLastRead += 100;
}