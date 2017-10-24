#include "MAX30105.h"
#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>

MAX30105 particleSensor;

#define debug Serial

const char *ssid = "UCInet Mobile Access";
unsigned int interval = 50 * 2 * 60;

void setup()
{
    int sampleSize = 0;
    debug.begin(115200);
    debug.println("Initializing...");

    // Initialize sensor
    //Use default I2C port, 400kHz speed
    if (!particleSensor.begin(Wire, I2C_SPEED_FAST))
    {
        debug.println("MAX30105 was not found. Please check wiring/power. ");
        while (1)
            ;
    }

    //LED Pulse Amplitude Configuration
    //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    //Default is 0x1F which gets us 6.4mA
    //powerLevel = 0x02; // 0.4mA - Presence detection of ~4 inch
    //powerLevel = 0x1F, 6.4mA - Presence detection of ~8 inch
    //powerLevel = 0x7F, 25.4mA - Presence detection of ~8 inch
    //powerLevel = 0xFF, 50.0mA - Presence detection of ~12 inch

    byte sampleRate = 50;      //Sample at 50 Hz
    byte ledBrightness = 0xFF; //Options: 0=Off to 255=50mA
    byte sampleAverage = 4;    //Options: 1, 2, 4, 8, 16, 32
    byte ledMode = 2;          //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
    int pulseWidth = 411;
    int adcRange = 2048;

    particleSensor.setPulseAmplitudeRed(0x1F);
    particleSensor.setPulseAmplitudeGreen(0x1F);

    particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);

    // WiFi setup
    WiFi.begin(ssid);

    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print("Attempting to connect...");
        delay(1000);
    }
}

void loop()
{

    // Gather the IR and RED data
    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;
        while (sampleSize < interval)
        {
            // Get the sensor data
            String data = "RED=" + (String)particleSensor.getRed() + "&" + "IR=" + (String)particleSensor.getIR();

            // Public IP of the EC2 instance is masked
            http.begin("http://xx.xxx.xxx.xxx/assignment2.php");
            http.addHeader("Content-Type", "application/x-www-form-urlencoded");
            http.POST(data);
            http.writeToStream(&Serial);

            sampleSize++;
        }
        http.end();
    }

    while (1)
    {
        delay(1000);
        wdt_reset();
    }
}