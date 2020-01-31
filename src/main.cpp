// This example sends data to multiple variables to
// Ubidots through HTTP protocol.

/****************************************
 * Include Libraries
 ****************************************/

#include <Arduino.h>

#include <Ubidots.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#include "DHTesp.h"
/****************************************
 * Define Instances and Constants
 ****************************************/
#define ESP_LED 2
#define NODE_LED 16
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define DHTPIN 14     // what digital pin the DHT22 is conected to
#define DHTTYPE DHT22 // There are multiple kinds of DHT sensors
DHTesp dht;

const char *UBIDOTS_TOKEN = "BBFF-z13DWfOsymYqZ10mPxMMmaVo0Dl7WW"; // Put here your Ubidots TOKEN

const char *WIFI_SSID = "lin";
const char *WIFI_PASS = "suupeersiixteeen";
const char *DEVICE_LABEL = "esprom";   // Replace with your device label
const char *VALUES_LABEL = "ledstate"; // Replace with your variable label
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
    pinMode(ESP_LED, OUTPUT);
    Serial.begin(9600);
    Wire.begin(D2, D1);
    lcd.init(); // initialize the lcd
    lcd.backlight();

    lcd.home();

    lcd.print("Hello, NodeMCU");

    dht.setup(DHTPIN, DHTesp::DHT22); // Connect DHT sensor to GPIO 17
    ubidots.wifiConnect(WIFI_SSID, WIFI_PASS);
    //ubidots.setDebug(true); // Uncomment this line for printing debug messages
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
    Serial.print(" *C \t ");
    Serial.print("Heat index: ");
    Serial.print(heatIndex);
    Serial.print(" *C ");

    lcd.clear();
    lcd.noBlink();
    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.setCursor(3, 0);
    lcd.print(tempC);

    lcd.setCursor(9, 0);
    lcd.print("H:");
    lcd.setCursor(11, 0);
    lcd.print(humidity);

    lcd.setCursor(0, 1);
    lcd.print("HI:");
    lcd.setCursor(4, 1);
    lcd.println(heatIndex);

    lcd.setContrast(255);

    ubidots.add("tempc", tempC); // Change for your variable name
    ubidots.add("humidity", humidity);
    ubidots.add("heatindexc", heatIndex);

    bool bufferSent = false;
    bufferSent = ubidots.send(); // Will send data to a device label that matches the device Id

    float webValue = ubidots.get(DEVICE_LABEL, VALUES_LABEL);

    lcd.setCursor(13, 1);
    lcd.print(webValue);
    digitalWrite(ESP_LED, webValue);

    if (bufferSent)
    {
        // Do something if values were sent properly
        Serial.println("Values sent by the device");
    }

    if (webValue != ERROR_VALUE)
    {
        Serial.print("Web Value: ");
        Serial.println(webValue);
    }
    delay(5000);
}