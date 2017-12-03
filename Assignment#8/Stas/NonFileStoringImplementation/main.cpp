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

const int PACKAGES = 300;
const int VALUES_PER_PACKAGE = 100;

unsigned long start = 0;
//unsigned long end = 600000;
unsigned long end = 60000;

void setup()
{
  Serial.begin(115200);
  Serial.println("Initializing...");

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

  // persistent connection code
  WiFi.mode(WIFI_STA);
  http.setReuse(true);

  start = millis();
  end += start;
}

int package_count = 0;

float x[VALUES_PER_PACKAGE];
float y[VALUES_PER_PACKAGE];
float z[VALUES_PER_PACKAGE];
unsigned long R[VALUES_PER_PACKAGE];
unsigned long IR[VALUES_PER_PACKAGE];

void loop()
{
  // persistent connection code
  HTTPClient http;
  http.begin(server_address);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  if(package_count < PACKAGES && millis() < end)
  {
    String accelerometer_data = "lis3dh=";
    String particle_sensor_data = "max30105=";
    int values_count = 0;

    if ((WiFiMulti.run() == WL_CONNECTED))
    {
      //USE_SERIAL.print("Connected to WiFi. Gathering sensor data.");
      
      while (values_count < VALUES_PER_PACKAGE)
      {
        //unsigned int time = 0;
        //time = micros();
        x[values_count] = myIMU.readFloatAccelX();
        y[values_count] = myIMU.readFloatAccelY();
        z[values_count] = myIMU.readFloatAccelZ();
        R[values_count] = particleSensor.getRed();
        IR[values_count] = particleSensor.getIR();
        values_count++;
        //time = micros() - time;
        //USE_SERIAL.println(time, DEC);
      }
      //unsigned int time = 0;
      //time = micros();
      values_count = 0;
      while(values_count < VALUES_PER_PACKAGE)
      {
        String data_x = String(x[values_count]);
        String data_y = String(y[values_count]);
        String data_z = String(z[values_count]);
        String data_R = String(R[values_count]);
        String data_IR = String(IR[values_count]);

        accelerometer_data = accelerometer_data + data_x + "," + data_y + "," + data_z + "\n";
        particle_sensor_data = particle_sensor_data + data_R + "," + data_IR + "\n";

        values_count++;
      }
      //USE_SERIAL.print("Package is almost ready!");
      //USE_SERIAL.println(accelerometer_data);
      //USE_SERIAL.println(particle_sensor_data);

      // Send data to the server after collecting VALUES_PER_PACKAGE values
      if (values_count % VALUES_PER_PACKAGE == 0)
      {
        // non-persistent connection code
        //HTTPClient http;
        //http.begin(server_address);
        //http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        int http_code_1 = http.POST(accelerometer_data);
        http.writeToStream(&Serial);
        //http.end();
        //http.begin(server_address);
        //http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        int http_code_2 = http.POST(particle_sensor_data);
        http.writeToStream(&Serial);

        //USE_SERIAL.printf("[HTTP] POST... code: %d\n", http_code_1);
        //USE_SERIAL.printf("[HTTP] POST... code: %d\n", http_code_2);

        // httpCode will be negative on error
        if (http_code_1 > 0 && http_code_2 > 0)
        {
          // HTTP header has been send and Server response header has been handled
          // file found at server
          if (http_code_1 == HTTP_CODE_OK && http_code_2 == HTTP_CODE_OK)
          {
            package_count++;
          }
        }
        else
        {
          USE_SERIAL.printf("[HTTP] POST... failed, error: %s error: %s\n", http.errorToString(http_code_1).c_str(), http.errorToString(http_code_2).c_str());
        }
        // non-persistent connection code
        //http.end();
      }
      //time = micros() - time;
      //USE_SERIAL.println(time, DEC);
      
    }
  }
  else{delay(100);}
  http.end();
}




