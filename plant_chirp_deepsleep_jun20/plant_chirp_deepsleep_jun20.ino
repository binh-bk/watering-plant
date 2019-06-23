// Binh Nguyen, June 20, 2019
// Soil sensor, ds18b20, 
//https://wemakethings.net/chirp/
//https://jsutton.co.uk/monitoring-houseplants-with-mqtt-and-the-esp8266/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>

WiFiClient espClient;
PubSubClient client(espClient);

#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#define ds18b20PIN D7

OneWire oneWire(ds18b20PIN);
DallasTemperature ds18b20(&oneWire);

#define CHIRPRESETPIN D6
#define blueLED D5

/*_____________________WIFI and MQTT_______________________*/
#define wifi_ssid "your_wifi_name"
#define wifi_password "your_wifi_password"
#define mqtt_server "your MQTT server IP"
#define mqtt_user "MQTT_user" 
#define mqtt_password "MTTQ_password"
#define mqtt_port 1883
#define publish_topic "sensors/balcony/plant"
#define SENSORNAME "plant"

/*_____________________GLOBAL VARIABLES______________________*/
float ds, bat;
uint16_t hum;
uint16_t lastSample;

/*_____________________START SETUP_______________________*/
void setup() {
  flashBuiltLED(100,3);
  Wire.begin();
  Serial.begin(9600);
  Serial.println("Starting Node named " + String(SENSORNAME));
  pinMode(blueLED, OUTPUT);
  ds18b20.begin();
  if (ds18b20.getDeviceCount() == 0) {
    Serial.printf("No DS18x20 found on pin %d.\n", ds18b20PIN);
    Serial.flush();
    delay(3000);
  }
  
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  flashLED(100, 3);
  
  ds18b20.requestTemperatures();
  ds = ds18b20.getTempCByIndex(0);
  hum = getMoistureReading();
  while (hum >=1000){
    hum = getMoistureReading();
  }
  int bat_ = analogRead(A0);
  bat = (bat_/900.0)*4.12;
  lastSample = round(millis()/1000);
  pushData();
  flashLED(200,3);
  delay(100); 
  ESP.deepSleep(12e8); //12*100 seconds == 20mins; 10^6=1 second
}

/*_____________________READ CHIRP SENSOR_______________________*/
void writeI2CRegister8bit(int addr, int value) {
  Wire.beginTransmission(addr);
  Wire.write(value);
  Wire.endTransmission();
}

unsigned int readI2CRegister16bit(int addr, int reg) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.endTransmission();
  delay(1100);
  Wire.requestFrom(addr, 2);
  unsigned int t = Wire.read() << 8;
  t = t | Wire.read();
  return t;
}

unsigned int getMoistureReading(){
  int moistureVal = readI2CRegister16bit(0x20, 0);
  Serial.print("ReadI2C Register: ");
  Serial.println(moistureVal);
  while ((moistureVal == -1)|| (moistureVal == 65535)){
    Serial.println("Bad reading from chirp, resetting...");
    digitalWrite(CHIRPRESETPIN, LOW);
    delay(500);
    digitalWrite(CHIRPRESETPIN, HIGH);
    flashLED(250, 3);
    moistureVal = readI2CRegister16bit(0x20, 0);
    Serial.print("ReadI2C Register: ");
    Serial.println(moistureVal);
  }
  
  return moistureVal;
}

/*_____________________MAIN LOOP_______________________*/
void loop() {
  // Do nothing here
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

/*_____________________SEND STATE_______________________*/
void pushData() {
  StaticJsonDocument<512> doc;
  doc["uptime"] = lastSample;
  doc["sensor"] = SENSORNAME;
  doc["ds18b20"] = ds;
  doc["hum"] = hum;
  doc["bat"] = bat;
  
  size_t len = measureJson(doc)+ 1;
  char payload[len];
  serializeJson(doc, payload, sizeof(payload));
    if (!client.connected()) {
    reconnect();
    delay(1000);
  }
  if (client.publish(publish_topic, payload, false)){
    Serial.println("Success: " + String(payload));
  } else {
    Serial.println("Failed to push: " + String(payload));
  }
}
/*_____________________START RECONNECT_______________________*/
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(SENSORNAME, mqtt_user, mqtt_password)) {
      Serial.println("connected");
    flashLED(50,3);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
/*_____________________ FLASH LED______________________*/
void flashLED(int onTime, int cycle){
  for (int i=0;i<cycle; i++){
    digitalWrite(blueLED, 1);
    delay(onTime);
    digitalWrite(blueLED, 0);
    delay(onTime);
  }
}
/*_____________________ FLASH BUILTIN LED______________________*/
void flashBuiltLED(int onTime, int cycle){
  for (int i=0;i<cycle; i++){
    digitalWrite(LED_BUILTIN, 0);
    delay(onTime);
    digitalWrite(LED_BUILTIN, 1);
    delay(onTime);
  }
}
