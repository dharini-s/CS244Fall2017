#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <SparkFunLIS3DH.h>
#include <SPI.h>

#include <ESP8266WiFiMulti.h>
#define USE_SERIAL Serial

ESP8266WiFiMulti WiFiMulti;
LIS3DH myIMU(SPI_MODE, 5);

// WiFi settings
const char *ssid = "UCInet Mobile Access";

const char *server_address = "http://**.***.***.***/assignment4.php";

const int PACKAGES = 60;
const int VALUES_PER_PACKAGE = 25;

void setup()
{
  Serial.begin(115200);
  Serial.println("Initializing...");

  // myIMU.settings.tempEnabled = 0;
  myIMU.settings.accelSampleRate = 1;  // Hz.  Can be: 0,1,10,25,50,100,200,400,1600,5000 Hz
  myIMU.settings.accelRange = 2;     // Max G force readable.  Can be: 2, 4, 8, 16
  myIMU.settings.xAccelEnabled = 1;
  myIMU.settings.yAccelEnabled = 1;
  myIMU.settings.zAccelEnabled = 1;

  //Call .begin() to configure the IMU
  myIMU.begin();

  for(uint8_t t = 4; t > 0; t--) {
      USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
      USE_SERIAL.flush();
      delay(1000);
  }

  USE_SERIAL.printf("TEST!");

  //WiFiMulti.addAP(ssid, password);
  WiFiMulti.addAP(ssid);
}

int package_count = 0;
bool gather_data = true;

float x[VALUES_PER_PACKAGE*PACKAGES];
float y[VALUES_PER_PACKAGE*PACKAGES];
float z[VALUES_PER_PACKAGE*PACKAGES];

void loop()
{
  if(package_count < PACKAGES)
  {
    String data_to_send = "data=";
    int values_count = 0;

    if ((WiFiMulti.run() == WL_CONNECTED))
    {
      USE_SERIAL.print("Connected to WiFi. Gathering sensor data.");
      
      while (gather_data && values_count < VALUES_PER_PACKAGE*PACKAGES)
      {
        x[values_count] = myIMU.readFloatAccelX();
        y[values_count] = myIMU.readFloatAccelY();
        z[values_count] = myIMU.readFloatAccelZ();
        values_count++;
      }

      gather_data = false;
      
      values_count = 0;
      while(values_count < VALUES_PER_PACKAGE)
      {
        String data_x = String(x[values_count+package_count*VALUES_PER_PACKAGE]);
        String data_y = String(y[values_count+package_count*VALUES_PER_PACKAGE]);
        String data_z = String(z[values_count+package_count*VALUES_PER_PACKAGE]);
        data_to_send += String(package_count*VALUES_PER_PACKAGE + values_count) + "," + data_x + "," + data_y + "," + data_z + "\n";
        values_count++;
      }
      USE_SERIAL.print("Package is almost ready!");

      // Send data to the server after collecting VALUES_PER_PACKAGE values
      if (values_count % VALUES_PER_PACKAGE == 0)
      {
        HTTPClient http;
        http.begin(server_address);
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        int httpCode = http.POST(data_to_send);

        if (httpCode > 0)
        {
          // HTTP header has been send and Server response header has been handled
          //USE_SERIAL.printf("[HTTP] POST... code: %d\n", httpCode);

          // file found at server
          if (httpCode == HTTP_CODE_OK)
          {
            String payload = http.getString();
            USE_SERIAL.println("[HTTP] POST... payload:");
            USE_SERIAL.println(payload);
            package_count++;
          }
        }
        else
        {
          USE_SERIAL.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
      }
    }
  }
  else
  {delay(100);}
}