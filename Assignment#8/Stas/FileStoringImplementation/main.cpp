#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFiMulti.h>
#include "MAX30105.h"
#include <Wire.h>
#include <SparkFunLIS3DH.h>
#include <SPI.h>
#include "FS.h"
#define USE_SERIAL Serial

ESP8266WiFiMulti WiFiMulti;
MAX30105 particleSensor;
LIS3DH myIMU;
HTTPClient http;

// WiFi settings
//const char *ssid = "UCInet Mobile Access";
const char *ssid = "";
const char *password = "";
const char *server_address = "http://**.***.***.***/assignment6.php";

const int VALUES_PER_PACKAGE = 100;
int NUMBER_OF_TRANSMISSIONS = 0;

unsigned long start = 0;
// 10 min in milliseconds
//unsigned long end = 600000;
unsigned long end = 150000;

File f1;
File f2;

void setup()
{
  Serial.begin(115200);
  Serial.println("Initializing...");

  // Begin SPIFFS
  SPIFFS.begin();
  SPIFFS.format();
  delay(10000);
  Serial.println("Spiffs formatted");

   // Initialize paritcle sensor
    // Use default I2C port, 400kHz speed
    if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
      Serial.println("MAX30105 was not found. Please check wiring/power. ");
      while (1);
  }

  byte sampleRate = 100; //Sample at 400 Hz
  byte ledBrightness = 0x02; //Options: 0=Off to 255=50mA
  byte sampleAverage = 1; //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  int pulseWidth = 411;
  int adcRange = 2048;

  particleSensor.setPulseAmplitudeRed(0x1F);
  particleSensor.setPulseAmplitudeGreen(0x1F);

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);

  // Initialize accelerometer
  myIMU.settings.accelSampleRate = 100; // Hz.  Can be: 0,1,10,25,50,100,200,400,1600,5000 Hz
  myIMU.settings.accelRange = 16; // Max G force readable.  Can be: 2, 4, 8, 16
  //myIMU.settings.xAccelEnabled = 1;
  //myIMU.settings.yAccelEnabled = 1;
  //myIMU.settings.zAccelEnabled = 1;
  //Call .begin() to configure the IMU
  myIMU.begin();

  for(uint8_t t = 4; t > 0; t--) {
      USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
      USE_SERIAL.flush();
      delay(1000);
  }

  USE_SERIAL.printf("TEST!");

  WiFiMulti.addAP(ssid, password);
  //WiFiMulti.addAP(ssid);

  // persistent connection code
  WiFi.mode(WIFI_STA);
  http.setReuse(true);
}

void collect_data()
{ 
    f1.print(myIMU.readFloatAccelX());
    f1.print(",");
    f1.print(myIMU.readFloatAccelY());
    f1.print(",");
    f1.print(myIMU.readFloatAccelZ());
    f1.print(",");
    f1.print(particleSensor.getRed());
    f1.print(",");
    f1.print(particleSensor.getIR());
    f1.print(",");
}

void loop()
{
  start = millis();
  end += start;
  int VALUES_STORED = 0;
  int PACKAGES_STORED = 0;
  int PACKAGES_READ = 0;

  f1 = SPIFFS.open("/f1.txt", "a");
    if (!f1) {
        Serial.println("File open failed");
    }
  while(millis() < end)
  {
    collect_data();
    VALUES_STORED++;
    if(VALUES_STORED % VALUES_PER_PACKAGE == 0)
    {PACKAGES_STORED++;}
    //USE_SERIAL.println(VALUES_STORED);
  }
  f1.close();
  USE_SERIAL.println("Finished storing data in file system.");
  
  f1 = SPIFFS.open("/f1.txt", "r");

  // persistent connection code
  HTTPClient http;
  http.begin(server_address);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  while(PACKAGES_READ < PACKAGES_STORED)
  {
    String accelerometer_data = "lis3dh=";
    String particle_sensor_data = "max30105=";
    int values_count = 0;

    if ((WiFiMulti.run() == WL_CONNECTED))
    {
      while(values_count < VALUES_PER_PACKAGE)
      {
        String data_x = f1.readStringUntil(',');
        String data_y = f1.readStringUntil(',');
        String data_z = f1.readStringUntil(',');
        String data_R = f1.readStringUntil(',');
        String data_IR = f1.readStringUntil(',');

        accelerometer_data = accelerometer_data + data_x + "," + data_y + "," + data_z + "\n";
        particle_sensor_data = particle_sensor_data + data_R + "," + data_IR + "\n";

        values_count++;
      }
      PACKAGES_READ++;

      // Send data to the server after collecting VALUES_PER_PACKAGE values
      if (values_count % VALUES_PER_PACKAGE == 0)
      {
        int http_code_1 = http.POST(accelerometer_data);
        http.writeToStream(&Serial);
        int http_code_2 = http.POST(particle_sensor_data);
        http.writeToStream(&Serial);

        //USE_SERIAL.printf("[HTTP] POST... code: %d\n", http_code_1);
        //USE_SERIAL.printf("[HTTP] POST... code: %d\n", http_code_2);

        // httpCode will be negative on error
        if (http_code_1 > 0 && http_code_2 > 0)
        {}
        else
        {
          USE_SERIAL.printf("[HTTP] POST... failed, error: %s error: %s\n", http.errorToString(http_code_1).c_str(), http.errorToString(http_code_2).c_str());
        }
      }
    }
  }

  f1.close();
  SPIFFS.remove("/f1.txt");

  // persistent connection code
  http.end();
  if(NUMBER_OF_TRANSMISSIONS < 4)
  {
    NUMBER_OF_TRANSMISSIONS++;
  }
  else
  {delay(1000000);}
  
}


