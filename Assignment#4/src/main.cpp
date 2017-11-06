#include "Arduino.h"
#include "ESP8266WiFi.h"
#include "ESP8266HTTPClient.h"
#include "Wire.h"
#include "SparkFunLIS3DH.h"
#include "SPI.h"

LIS3DH myIMU(SPI_MODE, 5);
String deviceName = "CS244";
const char *ssid = "UCInet Mobile Access";
static int SAMPLES = 1000;
const int BATCH = 100;
int count = 0;
int batchCount = 0;

float x[BATCH + 1];
float y[BATCH + 1];
float z[BATCH + 1];

void setup() {

  Serial.begin(115200);
  delay(1000);
  Serial.println("Start of program");

  WiFi.begin(ssid);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print("Attempting to connect...");
    delay(1000);
  }

  // myIMU.settings.tempEnabled = 0;
  myIMU.settings.accelSampleRate = 100;  //Hz.  Can be: 0,1,10,25,50,100,200,400,1600,5000 Hz
  // myIMU.settings.accelRange = 2;      //Max G force readable.  Can be: 2, 4, 8, 16
  // myIMU.settings.xAccelEnabled = 1;
  // myIMU.settings.yAccelEnabled = 1;
  // myIMU.settings.zAccelEnabled = 1;

  //Call .begin() to configure the IMU
  myIMU.begin();

}

void sendDataToServer(int offset) {
  String data = "data=";

  if(WiFi.status() == WL_CONNECTED) {
    for(int i = 0; i < BATCH; ++i)  {
      data = data + (offset + i) + "," + x[i] + "," + y[i] + "," + z[i] + "\n";
    }

    HTTPClient http;
    // Public IP of the EC2 instance is masked
    http.begin("http://**.***.***.***/assignment4.php");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.POST(data);
    http.writeToStream(&Serial);
    http.end();
  }
  delay(2000);
}

void loop()
{
  
  if(count < SAMPLES) {
    Serial.print(" X = ");
    x[batchCount] = myIMU.readFloatAccelX();
    Serial.println(x[batchCount]);
    Serial.print(" Y = ");
    y[batchCount] = myIMU.readFloatAccelY();
    Serial.println(y[batchCount]);
    Serial.print(" Z = ");
    z[batchCount] = myIMU.readFloatAccelZ();
    Serial.println(z[batchCount]);
    batchCount++;
    count++;

    if ((batchCount % BATCH) == 0) {
      int offset = count - BATCH;
      sendDataToServer(offset);
      batchCount = 0;
    }
  }
  else{
    delay(1000);
  }
}