
#include <Adafruit_BMP280.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <Wire.h>

const char *WIFI_SSID = "FikFok";
const char *WIFI_PASS = "12345678";
const char *MQTT_HOST = "10.10.10.102";
const char *MQTT_USER = "esp32user";
const char *MQTT_PASS = "Esp2569farm";
const char *MQTT_ID   = "ESP32_TeamNayok";
const char *TOPIC_ENV = "farm/plot/1/env";
const char *TOPIC_CMD = "farm/plot/1/cmd";

#define LED_PIN 2
#define I2C_SDA 21
#define I2C_SCL 22

Adafruit_BMP280 bmp;
WiFiClient wifi;
PubSubClient mqtt(wifi);
String led = "OFF";
unsigned long lastSend = 0;

void sendData() {
  float t = bmp.readTemperature();
  float p = bmp.readPressure() / 100.0;
  if (isnan(t) || isnan(p)) { t = 0; p = 0; }

  String json = "{";
  json += "\"team\":\"" + String(MQTT_ID) + "\",";
  json += "\"temp\":"   + String(t, 2)    + ",";
  json += "\"hum\":0,";
  json += "\"pres\":"   + String(p, 2)    + ",";
  json += "\"led\":\""  + led             + "\"";
  json += "}";

  mqtt.publish(TOPIC_ENV, json.c_str());
  Serial.println(json);
}

void onMessage(char *topic, byte *payload, unsigned int length) {
  String msg = "";
  for (int i = 0; i < length; i++) msg += (char)payload[i];

  if (msg == "ON" || msg == "1") {
    led = "ON";
    digitalWrite(LED_PIN, HIGH);
  } else {
    led = "OFF";
    digitalWrite(LED_PIN, LOW);
  }
  sendData();
}

void connectMqtt() {
  while (!mqtt.connected()) {
    if (mqtt.connect(MQTT_ID, MQTT_USER, MQTT_PASS)) {
      mqtt.subscribe(TOPIC_CMD);
    } else {
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  Wire.begin(I2C_SDA, I2C_SCL);
  if (!bmp.begin(0x76)) {
    Serial.println("Warning: ไม่เจอ BMP280 (แต่ระบบทำงานต่อ)");
  }

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  mqtt.setServer(MQTT_HOST, 1883);
  mqtt.setCallback(onMessage);
}

void loop() {
  if (!mqtt.connected()) connectMqtt();
  mqtt.loop();

  if (millis() - lastSend > 2000) {
    lastSend = millis();
    sendData();
  }
}
