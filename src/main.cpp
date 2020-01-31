// This example sends data to multiple variables to
// Ubidots through HTTP protocol.

/****************************************
 * Include Libraries
 ****************************************/

#include <Ubidots.h>
#include <Arduino.h>

#include "DHTesp.h"
/****************************************
 * Define Instances and Constants
 ****************************************/

#define DHTPIN 4      // what digital pin the DHT22 is conected to
#define DHTTYPE DHT22 // There are multiple kinds of DHT sensors
DHTesp dht;

const char *UBIDOTS_TOKEN = "BBFF-z13DWfOsymYqZ10mPxMMmaVo0Dl7WW"; // Put here your Ubidots TOKEN

const char *WIFI_SSID = "lin";
const char *WIFI_PASS = "suupeersiixteeen";
Ubidots ubidots(UBIDOTS_TOKEN, UBI_HTTP);

/****************************************
 * Auxiliar Functions
 ****************************************/

// Put here your auxiliar functions

/****************************************
 * Main Functions
 ****************************************/

void setup()
{
    Serial.begin(9600);
    dht.setup(DHTPIN, DHTesp::DHT22); // Connect DHT sensor to GPIO 17
    ubidots.wifiConnect(WIFI_SSID, WIFI_PASS);
    ubidots.setDebug(true); // Uncomment this line for printing debug messages
}

void loop()
{
    delay(dht.getMinimumSamplingPeriod());
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float humidity = dht.getHumidity();
    // Read temperature as Celsius (the default)
    float tempC = dht.getTemperature();

    // Compute heat index in Celsius (isFahreheit = false)
    float heatIndex = dht.computeHeatIndex(tempC, humidity, false);

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(tempC);
    Serial.print(" *C ");
    Serial.print("Heat index: ");
    Serial.print(heatIndex);
    Serial.print(" *C ");

    ubidots.add("tempc", tempC); // Change for your variable name
    ubidots.add("humidity", humidity);
    ubidots.add("heatindexc", heatIndex);

    bool bufferSent = false;
    bufferSent = ubidots.send(); // Will send data to a device label that matches the device Id

    if (bufferSent)
    {
        // Do something if values were sent properly
        Serial.println("Values sent by the device");
        Serial.println("HEEHEHEH");
    }

    delay(5000);
}