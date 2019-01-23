#include <SPI.h>
#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include "coap.h"
#include "DHT.h"

#define DHTTYPE DHT22
#define DHTPIN 0
DHT dht(DHTPIN, DHTTYPE);

#define LUMPIN 0

byte mac[] = { 0xec, 0xb1, 0xd7, 0x59, 0xfc, 0xf4 };

// UDP and CoAP class
EthernetUDP Udp;
Coap coap(Udp);


// GET dht temperature
void callback_temperature(CoapPacket &packet, IPAddress ip, int port){
  char response[200];
  
  // extract payload
  char p[packet.payloadlen + 1];
  memcpy(p, packet.payload, packet.payloadlen);
  p[packet.payloadlen] = 0;
  String message(p);

  float t = dht.readTemperature();

  if (isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    coap.sendResponse(ip, port, packet.messageid, (char*)"Error reading data from DHT22.");
    return;
  }
  
  Serial.println(t);

  
  sprintf(response, "Temp: %d °C", (int) t);

  /*
  int i;
  for(i=0; i < (int) packet.optionnum; i++){
    Serial.print("Option:");
    Serial.println(i);
    Serial.println(packet.options[i].number);  
    Serial.println(*(packet.options[i].buffer));  
  }*/

  if(packet.code==1){ 
    Serial.println("Received GET request for dht sensor.");
    coap.sendResponse(ip, port, packet.messageid, response);
  }
}


// GET dht humidite
void callback_humidite(CoapPacket &packet, IPAddress ip, int port){
  char response[200];
  
  // extract payload
  char p[packet.payloadlen + 1];
  memcpy(p, packet.payload, packet.payloadlen);
  p[packet.payloadlen] = 0;
  String message(p);

  float h = dht.readHumidity();

  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    coap.sendResponse(ip, port, packet.messageid, (char*)"Error reading data from DHT22.");
    return;
  }
  
  Serial.println(h);
  
  sprintf(response, "Humidite: %d%%", (int) h);
  
  if(packet.code==1){ 
    Serial.println("Received GET request for dht sensor.");
    coap.sendResponse(ip, port, packet.messageid, response);
  }
}


// GET lumiere
void callback_lumiere(CoapPacket &packet, IPAddress ip, int port){
  char response[200];
  
  // extract payload
  char p[packet.payloadlen + 1];
  memcpy(p, packet.payload, packet.payloadlen);
  p[packet.payloadlen] = 0;
  String message(p);

  int lum = analogRead(LUMPIN);
  
  Serial.println(lum);
  if(lum<=100){
    sprintf(response, "Nuit");
  }
  else if(lum<=200){
    sprintf(response, "Soir");
  }
  else{sprintf(response, "Jour");}
  
  if(packet.code==1){ 
    Serial.println("Received GET request for dht sensor.");
    coap.sendResponse(ip, port, packet.messageid, response);
  }
}


void setup() {
  //IPAddress myip;
  Serial.begin(9600);

  IPAddress myip(192, 168, 0, 1);
  Ethernet.begin(mac, myip);
  //myip = Ethernet.localIP();
  
  Serial.print("My ip: ");  
  Serial.println(myip);  

  Serial.println("Starting DHT sensor...");
  dht.begin();

  Serial.println("Starting endpoints...");  
  coap.server(callback_temperature, "temperature");
  coap.server(callback_humidite, "humidite");
  coap.server(callback_lumiere, "lumiere");

  coap.start();
  Serial.println("Ready.");
}

void loop() {
  coap.loop();
}
