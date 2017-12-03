#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <MAX30105.h>

#include <ESP8266WiFiMulti.h>
#define USE_SERIAL Serial

ESP8266WiFiMulti WiFiMulti;
MAX30105 particleSensor;


// WiFi settings
//const char *ssid = "UCInet Mobile Access";
const char *ssid = "VDC-Resident";
const char *password = "AC86fm!6";

const char *server_address = "http://100.67.82.193/http_post_handling_noforms.php";

const int PACKAGES = 120;
const int VALUES_PER_PACKAGE = 25;

void setup()
{
  Serial.begin(115200);
  Serial.println("Initializing...");

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }

  //particleSensor.setup(); //Configure sensor. Use 6.4mA for LED drive

  byte ledBrightness = 0xFF; //Options: 0=Off to 255=50mA (0x02, 0x1F, 0x7F, 0xFF)
  byte sampleAverage = 2; //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  byte sampleRate = 200; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411; //Options: 69, 118, 215, 411
  int adcRange = 2048; //Options: 2048, 4096, 8192, 16384
  
  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);

  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();

  for(uint8_t t = 4; t > 0; t--) {
      USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
      USE_SERIAL.flush();
      delay(1000);
  }

  USE_SERIAL.printf("TEST!");

  WiFiMulti.addAP(ssid, password);
  //WiFiMulti.addAP(ssid);
}

int j = 0;
bool gather_data = true;
unsigned long R[VALUES_PER_PACKAGE*PACKAGES];
unsigned long IR[VALUES_PER_PACKAGE*PACKAGES];

void loop()
{
  // The sensor is able to generate 50 samples per minute. However, we can't read the data out 
  // that quickly. In ideal world we'd be able to generate 6000 samples in 2 minutes. However, 
  // due to execution time constraints (string concatenation online) we can only generate 
  // (1000ms/160ms) * 120s ~= 750 samples in 2 minutes.

  // Solution: first read in the desired 6000 samples into arrays, then convert them to strings & concatenate.
  // Update: ran into a pandora box of glitches and bugs while attempting this approach. Attempt to debug
  // further, if that takes too long stick with the slow approach?

  // Update2: this approach had no effect on performance. It still takes 160ms to extract R & IR samples.
  // Update3: it was taking 160ms to generate an R & IR sample because each R & IR sample was an average of 
  // 4 samples. Adjust sample average (2) and sample rate (200).

  if(j < PACKAGES)
  {
    String data_to_send = "";
    int i = 0;

    if ((WiFiMulti.run() == WL_CONNECTED))
    {
      USE_SERIAL.print("Connected to WiFi. Gathering sensor data.");
      
      while (gather_data && i < VALUES_PER_PACKAGE*PACKAGES)
      //while (i < VALUES_PER_PACKAGE)
      {
        unsigned int time = 0;
        time = micros();

        R[i] = particleSensor.getRed();
        IR[i] = particleSensor.getIR();
        
        //USE_SERIAL.print(i);
        //USE_SERIAL.println();
        //USE_SERIAL.print(String(R[i]));
        //USE_SERIAL.println();

        time = micros() - time;
        USE_SERIAL.println(time, DEC);
        
        i++;
      }
      // only gather sensor data once
      gather_data = false;
      USE_SERIAL.print("Sampling complete!");
      
      i = 0;
      while(i < VALUES_PER_PACKAGE)
      {
        String data_R = String(R[i+j*VALUES_PER_PACKAGE]);
        String data_IR = String(IR[i+j*VALUES_PER_PACKAGE]);
        data_to_send += " R[" + data_R + "] IR[" + data_IR + "]\n";
        i++;
      }
      USE_SERIAL.print("Package is almost ready!");

      // Send data to the server after collecting VALUES_PER_PACKAGE R & IR values
      if (i % VALUES_PER_PACKAGE == 0)
      {

        HTTPClient http;

        http.begin(server_address); //HTTP

        http.addHeader("Content-Type", "application/x-www-form-urlencoded");

        //name0_4mA, name6_4mA, name25_4mA, name50mA
        int httpCode = http.POST("name50mA=" + data_to_send);

        // httpCode will be negative on error
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

            j++;
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