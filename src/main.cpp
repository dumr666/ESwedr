// This example sends data to multiple variables to
// Ubidots through HTTP protocol.

/****************************************
 * Include Libraries
 ****************************************/

#include <Arduino.h>

#include <Ubidots.h>
//#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#include <DHTesp.h>
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
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C

#define DHTPIN 14     // what digital pin the DHT22 is conected to
#define DHTTYPE DHT22 // There are multiple kinds of DHT sensors
DHTesp dht;

//const char *UBIDOTS_TOKEN = "BBFF-z13DWfOsymYqZ10mPxMMmaVo0Dl7WW"; // Put here your Ubidots TOKEN
const char *UBIDOTS_TOKEN = "BBFF-WKqMg6x7sbRwkmAuLoF9zQoRP19lTw"; // Put here your Ubidots TOKEN

const char *WIFI_SSID = "Skynet2G";
const char *WIFI_PASS = "SarahConnor";
const char *DEVICE_LABEL = "esprom";   // Replace with your device label
const char *VALUES_LABEL = "ledstate"; // Replace with your variable label
Ubidots ubidots(UBIDOTS_TOKEN, UBI_HTTP);

/****************************************
 * Auxiliar Functions
 ****************************************/


double computeDewPoint2(double celsius, double humidity);
double dewPointFast(double celsius, double humidity);
float computeHeatIndex(float temperature, float percentHumidity, bool isFahrenheit);
float convertCtoF(float c) { return c * 1.8 + 32; }
float convertFtoC(float f) { return (f - 32) * 0.55555; }


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
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1);
    }
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
        Serial.println(F("SSD1306 allocation failed"));
        for(;;); // Don't proceed, loop forever
    }
    
    // Clear the buffer
    display.clearDisplay();

    // Show the display buffer on the screen. You MUST call display() after
    // drawing commands to make them visible on screen!
    display.display();
    delay(2000);

    String ipAddress = ubidots.wifiConnect(WIFI_SSID, WIFI_PASS);
    Serial.println(ipAddress);
    
}

void loop()
{
    delay(dht.getMinimumSamplingPeriod());

    float temperature = bme.readTemperature();
    float pressure = bme.readPressure();
    float altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
    float humidity = bme.readHumidity();
    float dewPoint = computeDewPoint2(temperature, humidity);
    float heatIndex = computeHeatIndex(temperature, humidity, false);

    Serial.print(F("Temperature = "));
    Serial.print(temperature);
    Serial.println(" *C");
    Serial.print(F("Pressure = "));
    Serial.print(pressure);
    Serial.println(" hPa");
    Serial.print(F("Humidity = "));
    Serial.print(humidity);
    Serial.println(" %");
    Serial.print(F("Dew Point = "));
    Serial.print(dewPoint);
    Serial.println(" *C");
    Serial.print(F("Heat Index = "));
    Serial.print(heatIndex);
    Serial.println(" *C");
    Serial.print(F("Altitute = "));
    Serial.print(altitude);
    Serial.println(" m");
    
    

    ubidots.add("tempc", temperature); // Change for your variable name
    ubidots.add("humidity", humidity);
    ubidots.add("heatindexc", pressure);

    bool bufferSent = false;
    bufferSent = ubidots.send(); // Will send data to a device label that matches the device Id

    float webValue = ubidots.get(DEVICE_LABEL, VALUES_LABEL);

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

double computeDewPoint2(double celsius, double humidity)
{
        double RATIO = 373.15 / (273.15 + celsius);  // RATIO was originally named A0, possibly confusing in Arduino context
        double SUM = -7.90298 * (RATIO - 1);
        SUM += 5.02808 * log10(RATIO);
        SUM += -1.3816e-7 * (pow(10, (11.344 * (1 - 1/RATIO ))) - 1) ;
        SUM += 8.1328e-3 * (pow(10, (-3.49149 * (RATIO - 1))) - 1) ;
        SUM += log10(1013.246);
        double VP = pow(10, SUM - 3) * humidity;
        double T = log(VP/0.61078);   // temp var
        return (241.88 * T) / (17.558 - T);
}

double dewPointFast(double celsius, double humidity)
{
        double a = 17.271;
        double b = 237.7;
        double temp = (a * celsius) / (b + celsius) + log(humidity*0.01);
        double Td = (b * temp) / (a - temp);
        return Td;
}

/*!
 *  @brief  Compute Heat Index
 *  				Using both Rothfusz and Steadman's equations
 *					(http://www.wpc.ncep.noaa.gov/html/heatindex_equation.shtml)
 *  @param  temperature
 *          temperature in selected scale
 *  @param  percentHumidity
 *          humidity in percent
 *  @param  isFahrenheit
 * 					true if fahrenheit, false if celcius
 *	@return float heat index
 */

float computeHeatIndex(float temperature, float percentHumidity,
                            bool isFahrenheit) {
  float hi;

  if (!isFahrenheit)
    temperature = convertCtoF(temperature);

  hi = 0.5 * (temperature + 61.0 + ((temperature - 68.0) * 1.2) +
              (percentHumidity * 0.094));

  if (hi > 79) {
    hi = -42.379 + 2.04901523 * temperature + 10.14333127 * percentHumidity +
         -0.22475541 * temperature * percentHumidity +
         -0.00683783 * pow(temperature, 2) +
         -0.05481717 * pow(percentHumidity, 2) +
         0.00122874 * pow(temperature, 2) * percentHumidity +
         0.00085282 * temperature * pow(percentHumidity, 2) +
         -0.00000199 * pow(temperature, 2) * pow(percentHumidity, 2);

    if ((percentHumidity < 13) && (temperature >= 80.0) &&
        (temperature <= 112.0))
      hi -= ((13.0 - percentHumidity) * 0.25) *
            sqrt((17.0 - abs(temperature - 95.0)) * 0.05882);

    else if ((percentHumidity > 85.0) && (temperature >= 80.0) &&
             (temperature <= 87.0))
      hi += ((percentHumidity - 85.0) * 0.1) * ((87.0 - temperature) * 0.2);
  }

  return isFahrenheit ? hi : convertFtoC(hi);
}

