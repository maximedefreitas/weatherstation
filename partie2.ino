#include <SPI.h>
#include <Dhcp.h>
#include <Dns.h>
#include <WiFi101.h>
#include <WiFiUdp.h>
#include "coap.h"
#include "DHT.h"

#include <DHT_U.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>

#define DHTTYPE DHT22
#define DHTPIN 0 
DHT dht(DHTPIN, DHTTYPE);
#define LUM A0
unsigned long readTime;


// UDP and CoAP class
WiFiUDP Udp;
Coap coap(Udp);


//Wifi 
char ssid[] = "ES_1134"; //  your network SSID (name)
char pass[] = "00000000";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;
char server[] = "www.google.com";    // name address for Google (using DNS)
WiFiClient client;


// CoAP server endpoint URL
// coap-client -m get coap://(address)/(option)

// GET temperature
void callback_temp(CoapPacket &packet, IPAddress ip, int port){
  char response[200];
  char char_t;
  
  // extract payload
  char p[packet.payloadlen + 1];
  memcpy(p, packet.payload, packet.payloadlen);
  p[packet.payloadlen] = NULL;
  String message(p);

  float t = dht.readTemperature();

  if (isnan(t)) {
    Serial.println("Failed to read temperature from DHT sensor!");
    coap.sendResponse(ip, port, packet.messageid, "Error reading temperature from DHT22.");
    return;
  }
  
  Serial.println(t);
  //dtostrf(t, 2, 2, char_t);
  //dtostrf(h, 2, 0, char_h);  
  sprintf(response, "%d" , (int) t);

  if(packet.code==1){ 
    Serial.println("Received GET request for temperature from dht sensor.");
    coap.sendResponse(ip, port, packet.messageid, response);
  }
}

// GET humidity
void callback_hum(CoapPacket &packet, IPAddress ip, int port){
  char response[200];
  char char_h;
  
  // extract payload
  char p[packet.payloadlen + 1];
  memcpy(p, packet.payload, packet.payloadlen);
  p[packet.payloadlen] = NULL;
  String message(p);

  float h = dht.readHumidity();

  if (isnan(h)) {
    Serial.println("Failed to read humidity from DHT sensor!");
    coap.sendResponse(ip, port, packet.messageid, "Error reading humidity from DHT22.");
    return;
  }
  
  Serial.println(h);
  //dtostrf(h, 2, 0, char_h);
  
  sprintf(response, "%d", (int) h);

  if(packet.code==1){ 
    Serial.println("Received GET request for humidity from dht sensor.");
    coap.sendResponse(ip, port, packet.messageid, response);
  }
}

// GET luminosity
void callback_lum(CoapPacket &packet, IPAddress ip, int port){
  char response[200];
  
  // extract payload
  char p[packet.payloadlen + 1];
  memcpy(p, packet.payload, packet.payloadlen);
  p[packet.payloadlen] = NULL;
  String message(p);

  int l = analogRead(LUM);

  if (isnan(l)) {
    Serial.println("Failed to read from light sensor!");
    coap.sendResponse(ip, port, packet.messageid, "Error reading data from light sensor.");
    return;
  }
  
  Serial.println(l);
  if(l>500){  
    sprintf(response, "Jour");
  }
  else if(l>200){
    sprintf(response, "Soir");
  }
  else {
    sprintf(response, "Nuit");
  }
  

  if(packet.code==1){ 
    Serial.println("Received GET request for light sensor.");
    coap.sendResponse(ip, port, packet.messageid, response);
  }
}



void setup() {
  
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 5 seconds for connection:
    delay(5000);
  }
  
  Serial.println("Connected to wifi");
  Serial.print("Adresse IP :");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);

  coap.server(callback_temp, "temperature");
  coap.server(callback_hum , "humidite");
  coap.server(callback_lum, "luminosite");
  
  coap.start();
  Serial.println("Ready.");
}

void loop() {
  coap.loop();
}
