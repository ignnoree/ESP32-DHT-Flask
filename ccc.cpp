#include <WiFi.h>
#include "DHTesp.h"
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* serverUrl = "https://esp-32-dht-flask.vercel.app/data";

const int DHT_PIN = 14;
DHTesp dht;

String lastTemp = "";
String lastHum = "";

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();

  lcd.setCursor(2,0);
  lcd.print("Turning on...");
  delay(1000);
  lcd.clear();

  dht.setup(DHT_PIN, DHTesp::DHT22);
  Serial.println("DHT22 ready");
  lcd.setCursor(2,0);
  lcd.print("DHT22 Ready");
  delay(1000);
  lcd.clear();

  WiFi.begin(ssid, password);
  lcd.setCursor(0,0);
  lcd.print("Connecting WiFi");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    lcd.setCursor(15,1);
    lcd.print("."); // simple spinner
    attempts++;
  }

  lcd.clear();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected!");
    lcd.setCursor(0,0);
    lcd.print("WiFi Connected");
    lcd.setCursor(0,1);
    lcd.print(WiFi.localIP());
    delay(2000);
  } else {
    Serial.println("\nFailed to connect WiFi");
    lcd.setCursor(0,0);
    lcd.print("WiFi Failed");
  }
  lcd.clear();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, reconnecting...");
    lcd.setCursor(2,0);
    lcd.print("Disconnected");
    lcd.setCursor(1,1);
    lcd.print("Reconnecting...");
    WiFi.reconnect();
    delay(5000);
    return;
  }

  lcd.clear();
  TempAndHumidity data;
  int readAttempts = 0;
  do {
    data = dht.getTempAndHumidity();
    if (!isnan(data.temperature) && !isnan(data.humidity)) break;
    Serial.println("DHT read failed, retrying...");
    delay(2000);
    readAttempts++;
    if (readAttempts > 5) return;
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
  for(int i = 0; i < 3; i++) {
    Serial.printf("POST attempt %d...\n", i + 1);
    lcd.setCursor(0,1);
    lcd.print("Sending");
    for(int s=0;s<3;s++){
      lcd.print(".");
      delay(200);
    }
    httpCode = http.POST(payload);
    if (httpCode > 0) break;
  }

  lcd.clear();
  if (httpCode == HTTP_CODE_OK) {
    String response = http.getString();
    lcd.setCursor(0,0);
    lcd.print("Temp:");
    lcd.print(t,1);
    lcd.print("C");
    lcd.setCursor(0,1);
    lcd.print("Hum:");
    lcd.print(h,1);
    lcd.print("%");
    Serial.println("Server response: " + response);
  } else {
    lcd.setCursor(0,0);
    lcd.print("Send failed");
    lcd.setCursor(0,1);
    lcd.print("Code:");
    lcd.print(httpCode);
    Serial.println("Failed to connect to server");
  }

  http.end();
  delay(60000);
}
