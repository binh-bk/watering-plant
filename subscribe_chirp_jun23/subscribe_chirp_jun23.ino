// Binh Nguyen, June 23, 2019
//subscribe to humidity sensor
//NeoPixel VCC <--> 5V
// DI <---> GPIO 0
// red for dry and green for wet. blue color reserved for other stuffs

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

WiFiClient espClient;
PubSubClient client(espClient);
#define PIN 0
#define NUMPIXELS 1
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

/*_____________________WIFI and MQTT_______________________*/
#define wifi_ssid "wifi_ssid"
#define wifi_password "wifi_password"
#define mqtt_server "MQTT_IP"
#define mqtt_user "MQTT_user" 
#define mqtt_password "MQTT_password"
#define mqtt_port 1883
#define subscribe_topic "sensors/balcony/plant"
#define SENSORNAME "indicator"

uint16_t hum;
uint32_t uptime;
int r,g,b;

/*_____________________START SETUP_______________________*/
void setup() {
  Serial.begin(9600);
  Serial.println("Starting Node named " + String(SENSORNAME));
  pinMode(PIN, OUTPUT);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  pixels.begin();
  set_color(350); 
}


/*_____________________MAIN LOOP_______________________*/
void loop() {
  client.loop();
  if (!client.connected()){
    reconnect();
  }
  uptime = millis()/1000;
  off_color(uptime);
  delay(200);
}

/*______________        _ SETUP WIFI _        _______________*/
void setup_wifi() {
  delay(10);
  Serial.printf("Connecting to %s", wifi_ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password);
  delay(100); 
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    i++;
    Serial.printf(" %i ", i);
    if (i == 5){
      WiFi.mode(WIFI_STA);
      WiFi.begin(wifi_ssid, wifi_password);
      delay(1000);
    }
    if (i >=10){
      ESP.restart();
      Serial.println("Resetting ESP");
    }
  }
  Serial.printf("\nWiFi connected: \t");
  Serial.print(WiFi.localIP());
  Serial.print("\twith MAC:\t");
  Serial.println(WiFi.macAddress());
}
/*_____________________START CALLBACK_______________________*/
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
    Serial.print("] ");

  char message[length + 1];
  for (int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';
  Serial.println(message);

  if (!processJson(message)) {
    return;
  }
}

/*_____________________START PROCESS JSON_______________________*/
bool processJson(char* message) {
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
      Serial.println("process mesg failed");
      return false;
    };
  int newhum;
  if (doc.containsKey("sensor")) {
        if (doc["sensor"] == "plant") {
          newhum = int(doc["hum"]);
        }
    };

   if (((newhum - hum) >= 5) || ((hum-newhum)>=5)){
    hum = newhum;
    set_color(hum);
   }
   return true;
}

/*_____________________START RECONNECT_______________________*/
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(SENSORNAME, mqtt_user, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(subscribe_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}


/*_____________________FLASH COLOR_______________________*/
void off_color(int uptime){

  if ((uptime % 10) == 0){
    for (int i=0; i<3; i++){
      pixels.clear();
      delay(250);
      pixels.setPixelColor(0, pixels.Color(255, 255, 255));
      pixels.show();
    }
    set_color(hum);
  }
}
/*_____________________ NEOPIXEL WHEEL______________________*/
void set_color(int hum_){
  if ((hum_ < 280) || (hum_>550)){
    r=g=0;   
    b = 255;
  } else {
    r = map(hum_,280,550,255,1);
    g = 255-r;
    b = 0; 
  }
  Serial.print("r: ");
  Serial.print(r);
  Serial.print("\tg: ");
  Serial.print(g);
  Serial.print("\tb: ");
  Serial.println(b);
  pixels.clear();
  pixels.setPixelColor(0, pixels.Color(r, g, b));
  pixels.show();
}
