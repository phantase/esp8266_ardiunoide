#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

#define USE_SERIAL Serial

#include <PubSubClient.h>

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN            4         // Pin which is connected to the DHT sensor.
#define DHTTYPE           DHT22     // DHT 22 (AM2302)

//#define wifi_ssid "Livebox-993C"
//#define wifi_password = "FF6624C2C3962DFAF26E9237AF"

#define wifi_ssid "PiOT"
#define wifi_password = "IOT4RPI3"

#define mqtt_server "192.168.168.1"
#define mqtt_user "guest"
#define mqtt_password "guest"

#define temperature_topic "sensor/temperature"
#define humidity_topic "sensor/humidity"

ESP8266WiFiMulti WiFiMulti;
DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayMS;

bool reading_ok;

void setup() {

    USE_SERIAL.begin(115200);
    
    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

    for(uint8_t t = 4; t > 0; t--) {
        USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }

    WiFiMulti.addAP(wifi_ssid, wifi_password);

  dht.begin();
  Serial.println("DHTxx Unified Sensor Example");
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.println("Temperature");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" *C");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" *C");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" *C");  
  Serial.println("------------------------------------");
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.println("Humidity");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println("%");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println("%");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println("%");  
  Serial.println("------------------------------------");
  // Set delay between sensor readings based on sensor details.
  delayMS = sensor.min_delay / 1000;

reading_ok = true;

}

void loop() {

if( reading_ok ) {
  delay(10000);
} else {
  delay(1000);
  reading_ok = true;
}

double temperature;
double humidity;

  sensors_event_t event;  
  
  if( reading_ok ){
    // Get temperature event and print its value.
    dht.temperature().getEvent(&event);
    if (isnan(event.temperature)) {
      Serial.println("Error reading temperature!");
      reading_ok = false;
    }
    else {
      Serial.print("Temperature: ");
      Serial.print(event.temperature);
      temperature = event.temperature;
      reading_ok = true;
      Serial.println(" *C");
    }
  }
  
  if( reading_ok ){
    // Get humidity event and print its value.
    dht.humidity().getEvent(&event);
    if (isnan(event.relative_humidity)) {
      Serial.println("Error reading humidity!");
      reading_ok = false;
    }
    else {
      Serial.print("Humidity: ");
      Serial.print(event.relative_humidity);
      humidity = event.relative_humidity;
      Serial.println("%");
    }
  }

  if( reading_ok ){
    // wait for WiFi connection
    if((WiFiMulti.run() == WL_CONNECTED)) {

        HTTPClient http;

        USE_SERIAL.print("[HTTP] begin...\n");
        // configure traged server and url
        String url = "http://192.168.1.11/test.php?";
        String temperature_parameter = "&temperature=";
        String humidity_parameter = "&humidity=";
        http.begin(url+temperature_parameter+temperature+humidity_parameter+humidity); //HTTP

        USE_SERIAL.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        int httpCode = http.GET();

        // httpCode will be negative on error
        if(httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

            // file found at server
            if(httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                USE_SERIAL.println(payload);
            }
        } else {
            USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
    }
  }

}
