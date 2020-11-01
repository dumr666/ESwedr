// This example sends data to multiple variables to
// Ubidots through HTTP protocol.

/****************************************
 * Include Libraries
 ****************************************/

#include <Arduino.h>

#include <Ubidots.h>
//#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <EnvironmentCalculation.h>
/****************************************
 * Define Instances and Constants
 ****************************************/
#define ESP_LED 2
#define NODE_LED 16
//LiquidCrystal_I2C lcd(0x27, 16, 2);
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_I2CDevice.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C

//const char *UBIDOTS_TOKEN = "BBFF-z13DWfOsymYqZ10mPxMMmaVo0Dl7WW"; // Put here your Ubidots TOKEN
const char *UBIDOTS_TOKEN = "BBFF-z13DWfOsymYqZ10mPxMMmaVo0Dl7WW"; // Put here your Ubidots TOKEN

const char *WIFI_SSID = "Skynet2G";
const char *WIFI_PASS = "SarahConnor";
const char *DEVICE_LABEL = "esprom";   // Replace with your device label
const char *VALUES_LABEL = "ledstate"; // Replace with your variable label
String ipAddress = "-";
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
    Serial.println(F("Testing connectivity of BME280"));
    bool status = bme.begin(0x76);
    if (!status)
    {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1)
            ;
    }
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ; // Don't proceed, loop forever
    }
    // Clear the buffer
    display.clearDisplay();

    // Draw a single pixel in white
    display.drawPixel(10, 10, WHITE);
    // Show the display buffer on the screen. You MUST call display() after
    // drawing commands to make them visible on screen!
    display.display();
    delay(2000);
    bool wifiConnected = ubidots.wifiConnect(WIFI_SSID, WIFI_PASS);
    Serial.println(ipAddress);
}

void loop()
{
    float temperature = bme.readTemperature();
    float pressureHpa = bme.readPressure();
    
    float pressure = pressureHpa / 100;

    float altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
    float humidity = bme.readHumidity();
    float dewPoint = computeDewPoint2(temperature, humidity);
    float heatIndex = computeHeatIndex(temperature, humidity, false);

    printEnvDataToSerial(temperature, pressure, humidity, dewPoint, heatIndex, altitude);

    ubidots.add("tempc", temperature); // Change for your variable name
    ubidots.add("humidity", humidity);
    ubidots.add("heatindexc", heatIndex);
    ubidots.add("pressure", pressure);
    ubidots.add("altitude", altitude);
    ubidots.add("dewpoint", dewPoint);

    display.clearDisplay();
    display.setTextSize(1);      // Normal 1:1 pixel scale
    display.setTextColor(WHITE); // Draw white text
    display.setCursor(0,0);
    //display.setTextSize(2);
    display.print("Temp:       ");
    display.print(temperature);
    display.println(" *C");
    display.print("Humidity:   ");
    display.print(humidity);
    display.println(" %");
    display.print("Heat Index: ");
    display.print(heatIndex);
    display.println(" *C");
    display.print("Dew Point:  ");
    display.print(dewPoint);
    display.println(" *C");
    display.print("Pressure:   ");
    display.print(pressure);
    display.println(" mb");
    display.print("Altitude:   ");
    display.print(altitude);
    display.println(" m");
    display.display();

    bool bufferSent = false;
    bufferSent = ubidots.send(); // Will send data to a device label that matches the device Id

    // float webValue = ubidots.get(DEVICE_LABEL, VALUES_LABEL);

    // digitalWrite(ESP_LED, webValue);

    if (bufferSent)
    {
        // Do something if values were sent properly
        Serial.println("Values sent by the device");
    }

    // if (webValue != ERROR_VALUE)
    // {
    //     Serial.print("Web Value: ");
    //     Serial.println(webValue);
    // }

    delay(5000);
}

