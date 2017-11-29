/**
 * Program to gather PPG sensor and accelerometer data using the ESP8266 board,
 * store the data in memory and send it to the server
 * 
 * Created on 26 Nov 2017
 * By dharini-s
 */
#include "MAX30105.h"
#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include "SparkFunLIS3DH.h"
#include "SPI.h"
#include "FS.h"

MAX30105 particleSensor;
LIS3DH myIMU;

#define debug Serial

const char *ssid = "UCInet Mobile Access";
const String URL = "http://***.***.***.***/assignment6.php";

const int BATCH = 100;
int batchCount = 0;
unsigned long start = 0;
// 10 min in milliseconds
unsigned long end = 600000;
int count = 0;
File f1;
File f2;

float x[BATCH + 1];
float y[BATCH + 1];
float z[BATCH + 1];
String ppg_data[BATCH + 1];

void setup()    {
    debug.begin(115200);
    debug.println("Initializing...");

    // Begin SPIFFS
    SPIFFS.begin();
    SPIFFS.format();
    delay(10000);
    Serial.println("Spiffs formatted");

    // Initialize paritcle sensor
    // Use default I2C port, 400kHz speed
    if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
        debug.println("MAX30105 was not found. Please check wiring/power. ");
        while(1);
    }

    byte sampleRate = 200; //Sample at 250 Hz
    byte ledBrightness = 0x02; //Options: 0=Off to 255=50mA
    byte sampleAverage = 4; //Options: 1, 2, 4, 8, 16, 32
    byte ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
    int pulseWidth = 411;
    int adcRange = 2048;

    particleSensor.setPulseAmplitudeRed(0x1F);
    particleSensor.setPulseAmplitudeGreen(0x1F);

    particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);

    // Initialize accelerometer
    myIMU.settings.accelSampleRate = 150;
    myIMU.settings.accelRange = 16;
    myIMU.begin();
    
    start = millis();
    end += start;
}

void sendDataToServer() {
    f1 = SPIFFS.open("/f1.txt", "r");
    f2 = SPIFFS.open("/f2.txt", "r");
    // WiFi setup
    // WiFi.persistent(false);
    // WiFi.mode(WIFI_OFF);
    // WiFi.mode(WIFI_STA);
    if (WiFi.status() != WL_CONNECTED) {
        WiFi.begin(ssid);
     }

    // Serial.println("After wifi begin");

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print("Attempting to connect...");
        delay(1000);
    }
    if (WiFi.status() == WL_CONNECTED) {  
        HTTPClient http;
        String accelerometer_data = "lis3dh=";
        String particle_sensor_data = "max30105=";
        String s1 = "";
        String s2 = "";

        int cnt2 = 0;
        while (f2.available())   {
            String s = f2.readStringUntil('\n');
            s2 += s + "\n";
            cnt2++;
            if (cnt2 == 100){
                cnt2 = 0;
                particle_sensor_data += s2;
                // Serial.println("PPG data:" + particle_sensor_data);
                http.begin(URL);
                http.addHeader("Content-Type", "application/x-www-form-urlencoded");
                http.POST(particle_sensor_data);
                http.writeToStream(&Serial);
                s2= "";
            }     
        }
        f2.close();

        int cnt1 = 0;
        while (f1.available())   {
            String s = f1.readStringUntil('\n');
            s1 += s + "\n";
            cnt1++;
            if (cnt1 == 100){
                cnt1 = 0;
                accelerometer_data += s1;
                http.begin(URL);
                http.addHeader("Content-Type", "application/x-www-form-urlencoded");
                http.POST(accelerometer_data);
                http.writeToStream(&Serial);
                s1= "";
            }
        }
        f1.close();
        SPIFFS.remove("/f1.txt");
        SPIFFS.remove("/f2.txt");

        http.end();
        
    }
    else    {
        Serial.println("Not connected to WiFi.");
    }
}
    
void writeToFile()  {
    // Open file for writing
    f1 = SPIFFS.open("/f1.txt", "a");
    f2 = SPIFFS.open("/f2.txt", "a");
    if (!f1 || !f2) {
        Serial.println("File open failed");
    }
    String accelerometer_data = "";
    String particle_sensor_data = "";

    // write to file
    for(int i = 0; i < BATCH; i++)  {
        accelerometer_data = accelerometer_data + x[i] + "," + y[i] + "," + z[i] + "\n";
        particle_sensor_data = particle_sensor_data + ppg_data[i] + "\n";
    }

    f1.print(accelerometer_data);
    f2.print(particle_sensor_data);
    f1.close();
    f2.close();
}

void loop() {
    // Gather the IR and RED data, and accelerometer data  
        if (millis() < end) {
            // Get the PPG sensor data
            ppg_data[batchCount] = (String)particleSensor.getRed() + "," + (String)particleSensor.getIR();
            // Get the accelerometer data
            x[batchCount] = myIMU.readFloatAccelX();
            y[batchCount] = myIMU.readFloatAccelY();
            z[batchCount] = myIMU.readFloatAccelZ();
            
            batchCount++;
            // Serial.println(batchCount);
            if ((batchCount % BATCH) == 0) {
                writeToFile();
                batchCount = 0;
                count++;
                if(count == 45) {
                    sendDataToServer();
                    count = 0;
                }
            }
        }
        else {
            Serial.println("Sent data to server");
            wdt_reset();
            while(1)    {
                delay(1000);
            }
        }
}