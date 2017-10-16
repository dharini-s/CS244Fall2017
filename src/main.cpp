#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

String deviceName = "CS244";
// WiFi settings
const char *ssid = "UCInet Mobile Access";

void setup()  {
  // Initialize serial for debugging
  Serial.begin(115200);
  Serial.println("Start of program");
  
  WiFi.begin(ssid);

  while ( WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect...");
    delay(1000);
  }
}

void loop() {
  String data = "key=Space:%20the%20final%20frontier";
  
  if(WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("http://**.***.***.***/hello.php");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.POST(data);
    http.writeToStream(&Serial);
    http.end();
  }
  delay(3000);
}