
#include "MAX30105.h"
#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include "SparkFunLIS3DH.h"
#include "SPI.h"

MAX30105 particleSensor;
LIS3DH myIMU;

#define debug Serial

const char *ssid = "UCInet Mobile Access";

const int BATCH = 100;
int count = 0;
int batchCount = 0;
unsigned long start = 0;
// 10 min in milliseconds
unsigned long end = 600000;

float x[BATCH + 1];
float y[BATCH + 1];
float z[BATCH + 1];
String ppg_data[BATCH + 1];

void setup()    {
    debug.begin(115200);
    debug.println("Initializing...");

    // Initialize paritcle sensor
    // Use default I2C port, 400kHz speed
    if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
        debug.println("MAX30105 was not found. Please check wiring/power. ");
        while (1);
    }

    byte sampleRate = 250; //Sample at 250 Hz
    byte ledBrightness = 0xFF; //Options: 0=Off to 255=50mA
    byte sampleAverage = 4; //Options: 1, 2, 4, 8, 16, 32
    byte ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
    int pulseWidth = 411;
    int adcRange = 2048;

    particleSensor.setPulseAmplitudeRed(0x1F);
    particleSensor.setPulseAmplitudeGreen(0x1F);

    particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);

    // Initialize accelerometer
    myIMU.settings.accelSampleRate = 250;
    myIMU.settings.accelRange = 16;
    myIMU.begin();

    // WiFi setup
    WiFi.begin(ssid);

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print("Attempting to connect...");
        delay(1000);
    }
    start = millis();
    end += start;
}

void sendDataToServer() {
    String accelerometer_data = "lis3dh=";
    String particle_sensor_data = "max30105=";
    
        for (int i = 0; i < BATCH; ++i) {
            accelerometer_data = accelerometer_data + x[i] + "," + y[i] + "," + z[i] + "\n";
            particle_sensor_data = particle_sensor_data + ppg_data[i] + "\n";
        }

        HTTPClient http;
        // Public IP of the EC2 instance is masked
        http.begin("http://xx.xxx.xxx.xxx/assignment6.php");
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        http.POST(accelerometer_data);
        http.writeToStream(&Serial);
        http.POST(particle_sensor_data);
        http.writeToStream(&Serial);
        http.end();
}

void loop() {
    // Gather the IR and RED data, and accelerometer data
    if (WiFi.status() == WL_CONNECTED) {
        if (millis() < end) {
            // Get the PPG sensor data
            ppg_data[batchCount] = (String)particleSensor.getRed() + "," + (String)particleSensor.getIR();

            // Get the accelerometer data
            x[batchCount] = myIMU.readFloatAccelX();
            y[batchCount] = myIMU.readFloatAccelY();
            z[batchCount] = myIMU.readFloatAccelZ();
            
            batchCount++;
            count++;

            if ((batchCount % BATCH) == 0) {
                sendDataToServer();
                batchCount = 0;
            }
        }
        else {
            Serial.println("End");
            delay(1000);
            wdt_reset();
        }
    }
}