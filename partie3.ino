#include <SPI.h>
#include <Dhcp.h>
#include <Dns.h>
#include <WiFi101.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>

#include <DHT_U.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>

#define DHTTYPE DHT22
#define DHTPIN 0 
DHT dht(DHTPIN, DHTTYPE);
#define LUM A0
unsigned long readTime;

//Wifi 
const char* ssid = "ES_1134";
const char* password = "00000000";
const char* mqtt_server = "192.168.43.223";
int keyIndex = 0;            // your network key Index number (needed only for WEP)



WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  client.publish("test", "Connexion établie");
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
    Serial.println();
}



void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("test", "Connexion établi");

      
    } 
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


// GET luminosity
void luminosite(){
  
  char reponse[50];
  int l = analogRead(LUM);

  if (isnan(l)) {
    Serial.println("Failed to read from light sensor!");
    sprintf(reponse, "erreur lecture capteur");
    client.publish("luminosite", reponse);
    return;
  }
  
  Serial.println(l);
  if(l>500){  
    sprintf(reponse, "Jour");
  }
  else if(l>110){
    sprintf(reponse, "Soir");
  }
  else {
    sprintf(reponse, "Nuit");
  }
  
  client.publish("luminosite", reponse);

  sprintf(reponse, "%d", l);
  client.publish("luminosite2", reponse);
  
}


// GET humidity
void humidite(){
  
  char reponse[50];
  char char_h;
  
  float h = dht.readHumidity();

  if (isnan(h)) {
    Serial.println("Failed to read humidity from DHT sensor!");
    sprintf(reponse, "erreur lecture capteur");
    client.publish("humidite", reponse);
    return;
  }
  
  Serial.println(h);
  //dtostrf(h, 2, 0, char_h);
  
  sprintf(reponse, "%d", (int) h);
  client.publish("humidite", reponse);
  
}

// GET temperature
void temperature(){
  char reponse[50];
  float t = dht.readTemperature();

  if (isnan(t)) {
    Serial.println("Failed to read temperature from DHT sensor!");
    client.publish("temperature", reponse);
    return;
  }
  
  Serial.println(t);
  //dtostrf(t, 2, 2, char_t);
  //dtostrf(h, 2, 0, char_h);  
  sprintf(reponse, "%d" , (int) t);

  client.publish("temperature", reponse);

}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    luminosite();
    humidite();
    temperature();
  }
}
