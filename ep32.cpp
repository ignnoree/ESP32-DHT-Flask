#include <WiFi.h>
#include "DHTesp.h"
#include <HTTPClient.h>


const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* serverUrl = "https://esp-32-dht-flask.vercel.app/data";

const int DHT_PIN = 14;
DHTesp dht;

void setup() {
  Serial.begin(115200);
  delay(1000);

  dht.setup(DHT_PIN, DHTesp::DHT22);
  Serial.println("DHT22 ready");


  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to WiFi");
  }
}

void loop() {

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, reconnecting...");
    WiFi.reconnect();
    delay(5000);
    return;
  }

  TempAndHumidity data;
  int readAttempts = 0;
  

  do {
    data = dht.getTempAndHumidity();
    if (!isnan(data.temperature) && !isnan(data.humidity)) break;
    Serial.println("DHT read failed, retrying...");
    delay(2000);
    readAttempts++;
    if (readAttempts > 5) {
      Serial.println("Max DHT read attempts reached");
      return;
    }
  } while (true);

  float t = data.temperature;
  float h = data.humidity;

  String payload = String("{\"temperature\":") + String(t,1) + 
                   String(",\"humidity\":") + String(h,1) +
                   String(",\"device\":\"esp32-wokwi\"}");

  Serial.println("Sending data: " + payload);


  HTTPClient http;

  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("User-Agent", "ESP32-Wokwi");

  http.setTimeout(10000);
  
  int httpCode = 0;
  for(int i = 0; i < 3; i++) { // retry 3 times
    Serial.printf("POST attempt %d...\n", i + 1);
    httpCode = http.POST(payload);
    
    if (httpCode > 0) {
      Serial.printf("HTTP Response code: %d\n", httpCode);
      break;
    } else {
      Serial.printf("POST failed, error: %s\n", http.errorToString(httpCode).c_str());
      if (i < 2) { // Don't delay after last attempt
        Serial.println("Retrying in 2s...");
        delay(2000);
      }
    }
  }

  if (httpCode == HTTP_CODE_OK) {
    String response = http.getString();
    Serial.println("Server response: " + response);
  } else if (httpCode > 0) {
    String response = http.getString();
    Serial.printf("HTTP Error %d: %s\n", httpCode, response.c_str());
  } else {
    Serial.println("Failed to connect to server after 3 attempts");
  }
  
  http.end();
  delay(10000); 
}