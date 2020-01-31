#include <Arduino.h>

#include <DHTesp.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Losant.h>

#define DHTPIN 4     // what digital pin the DHT22 is conected to
#define DHTTYPE DHT22   // There are multiple kinds of DHT sensors

DHTesp dht;

// WiFi credentials.
const char* WIFI_SSID = "Skynet";
const char* WIFI_PASS = "SarahConnor";

// Losant credentials.
const char* LOSANT_DEVICE_ID = "5e30a98d91f4d70006d4fd45";
const char* LOSANT_ACCESS_KEY = "3173315e-3b81-4a1d-91e8-7038bac35c0c";
const char* LOSANT_ACCESS_SECRET = "b114021d0e203e9d0cdb878d54b7e00e95402e2dfac1ffe2cc0c46dda2ccbed8";


WiFiClient wifiClient;

LosantDevice device(LOSANT_DEVICE_ID);

void connect() {

  // Connect to Wifi.
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  // WiFi fix: https://github.com/esp8266/Arduino/issues/2186
  WiFi.persistent(false);
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  unsigned long wifiConnectStart = millis();

  while (WiFi.status() != WL_CONNECTED) {
    // Check to see if
    if (WiFi.status() == WL_CONNECT_FAILED) {
      Serial.println("Failed to connect to WIFI. Please verify credentials: ");
      Serial.println();
      Serial.print("SSID: ");
      Serial.println(WIFI_SSID);
      Serial.print("Password: ");
      Serial.println(WIFI_PASS);
      Serial.println();
    }

    delay(500);
    Serial.println("...");
    // Only try for 5 seconds.
    if(millis() - wifiConnectStart > 5000) {
      Serial.println("Failed to connect to WiFi");
      Serial.println("Please attempt to send updated configuration parameters.");
      return;
    }
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.print("Authenticating Device...");
  HTTPClient http;
  http.begin("http://api.losant.com/auth/device");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Accept", "application/json");

  /* Create JSON payload to sent to Losant
   *
   *   {
   *     "deviceId": "575ecf887ae143cd83dc4aa2",
   *     "key": "this_would_be_the_key",
   *     "secret": "this_would_be_the_secret"
   *   }
   *
   */

  //StaticJsonDocument<200> jsonBuffer;
  DynamicJsonDocument root(200);
  //JsonObject root = jsonBuffer.createObject();
  root["deviceId"] = LOSANT_DEVICE_ID;
  root["key"] = LOSANT_ACCESS_KEY;
  root["secret"] = LOSANT_ACCESS_SECRET;
  String buffer;
  //serializeJson(buffer, Serial);

  int httpCode = http.POST(buffer);

  if(httpCode > 0) {
      if(httpCode == HTTP_CODE_OK) {
          Serial.println("This device is authorized!");
      } else {
        Serial.println("Failed to authorize device to Losant.");
        if(httpCode == 400) {
          Serial.println("Validation error: The device ID, access key, or access secret is not in the proper format.");
        } else if(httpCode == 401) {
          Serial.println("Invalid credentials to Losant: Please double-check the device ID, access key, and access secret.");
        } else {
           Serial.println("Unknown response from API");
        }
      }
    } else {
        Serial.println("Failed to connect to Losant API.");

   }

  http.end();

  // Connect to Losant.
  Serial.println();
  Serial.print("Connecting to Losant...");

  device.connect(wifiClient, LOSANT_ACCESS_KEY, LOSANT_ACCESS_SECRET);

  while(!device.connected()) {
    delay(500);
    
    Serial.println(device.mqttClient.state()); // HERE
    Serial.print(".");
  }

  Serial.println("Connected!");
  Serial.println();
  Serial.println("This device is now ready for use!");
}

void setup() {
  
  dht.setup(4, DHTesp::DHT22); // Connect DHT sensor to GPIO 17
  Serial.begin(9600);
  Serial.setTimeout(2000);
  
  // Wait for serial to initialize.
  while(!Serial) { }

  Serial.println("Device Started");
  Serial.println("-------------------------------------");
  Serial.println("Running DHT!");
  Serial.println("-------------------------------------");

  connect();
}

void report(double humidity, double tempC,  double heatIndexC) {
  
  DynamicJsonDocument data(500);
  /*
  root["deviceId"] = LOSANT_DEVICE_ID;
  root["key"] = LOSANT_ACCESS_KEY;
  root["secret"] = LOSANT_ACCESS_SECRET;
  String buffer;
*/
  //StaticJsonBuffer<500> jsonBuffer;
  //JsonObject& root = jsonBuffer.createObject();
  data["humidity"] = humidity;
  data["tempC"] = tempC;
  data["heatIndexC"] = heatIndexC;
  device.sendState(data);
  Serial.println("Reported!");
}

int timeSinceLastRead = 0;
void loop() {
   bool toReconnect = false;

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Disconnected from WiFi");
    toReconnect = true;
  }

  if (!device.connected()) {
    Serial.println("Disconnected from MQTT");
    Serial.println(device.mqttClient.state());
    toReconnect = true;
  }

  if (toReconnect) {
    connect();
  }

  device.loop();

  // Report every 2 seconds.
  
    delay(dht.getMinimumSamplingPeriod());
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.getHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.getTemperature();


    // Compute heat index in Celsius (isFahreheit = false)
    float hic = dht.computeHeatIndex(t, h, false);

    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print(" *C ");
    Serial.print("Heat index: ");
    Serial.print(hic);
    Serial.print(" *C ");
    report(h, t, hic);

    timeSinceLastRead = 0;
  delay(600);
  timeSinceLastRead += 600;
}