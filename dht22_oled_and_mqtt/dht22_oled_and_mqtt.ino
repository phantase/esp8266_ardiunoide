// for OLED display
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
// for DHT sensor
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
// for WiFi & MQTT connection
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define DHTPIN            2         // Pin which is connected to the DHT sensor.
#define DHTTYPE           DHT22     // DHT 22 (AM2302)

DHT_Unified dht(DHTPIN, DHTTYPE);   // Initialize the DHT

SSD1306  display(0x3c, D3, D5);     // Initialize the OLED display using Wire library

uint32_t delayMS;                   // Will receive the delay between message (in ms)

// Information about local network & MQTT server (in this case, I use an Raspberry Pi 3 with built in WiFi and MQTT brooker on it)
const char* ssid = "PiOT";
const char* password = "IOT4RPI3";
const char* mqtt_server = "192.168.168.1";

WiFiClient espClient;               // for WiFi connection
PubSubClient client(espClient);     // for MQTT access (publication & subscription)

long lastMsg = 0;
int value = 0;
char msg[50];
char msg2[10];
const char* sensor_id = "ORL_Bedroom";

void setup() {
  Serial.begin(115200);
  Serial.println();

  Serial.println("Initializing OLED");
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.clear();

  setup_wifi();

  display.drawString(0,30,"Initializing MQTT");
  display.display();
  Serial.println("Initializing MQTT");
  client.setServer(mqtt_server, 1883);

  display.drawString(0,40,"Initializing DHT");
  display.display();
  Serial.println("Initializing DHT");
  dht.begin();
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  delayMS = sensor.min_delay / 1000;  // Set delay between sensor readings based on sensor details.

  sprintf(msg, "Refresh delay %d ms",delayMS);
  display.drawString(0,50,msg);
  display.display();
  Serial.print("Refresh delay: ");
  Serial.print(delayMS);
  Serial.println(" ms");

}

void setup_wifi() {
  display.clear();
  display.drawString(0,0,"Init WiFi...");
  display.display();
  Serial.println("Initializing WiFi");
  
  delay(10);
  // We start by connecting to a WiFi network
  sprintf(msg, "Connecting to %s",ssid);
  display.drawString(0,10,msg);
  display.display();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  value = 0;
  while (WiFi.status() != WL_CONNECTED) {
    ++value;
    display.drawProgressBar(0,32,120,10,value);
    display.display();
    delay(500);
    Serial.print(".");
  }

  display.clear();
  display.drawString(0,0,"WiFi connected");
  display.drawString(0,10,"IP address: ");
  display.drawString(0,20,String(WiFi.localIP()));
  display.display();
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      snprintf(msg, 75, "phantase/sensors/%s/message",sensor_id);
      client.publish(msg, "connection");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > delayMS){
    lastMsg = now;
    
    display.clear();
  
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 0, "Temperature:");
    display.drawString(0, 32, "Humidity:");
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
  
    sensors_event_t event;
    
    dht.temperature().getEvent(&event);
    if (isnan(event.temperature)) {
      display.drawString(128, 16, "Reading error");
      snprintf(msg, 75, "phantase/sensors/%s/message",sensor_id);
      client.publish(msg, "error reading temperature");
      Serial.println("Error reading temperature");
    }
    else {
      display.setTextAlignment(TEXT_ALIGN_LEFT);
      display.drawString(100, 16, "°C");
      display.setTextAlignment(TEXT_ALIGN_RIGHT);
      display.drawString(100, 16, String(event.temperature));
      snprintf(msg , 75, "phantase/sensors/%s/temperature",sensor_id);
      int temp1 = (event.temperature - (int)event.temperature)*100;
      snprintf(msg2, 10, "%d.%d",(int)event.temperature,temp1);
      client.publish(msg, msg2);
      Serial.print("Temperature: ");
      Serial.println(event.temperature);
    }
  
    dht.humidity().getEvent(&event);
    if (isnan(event.relative_humidity)) {
      display.drawString(128, 16, "Reading error");
      snprintf(msg, 75, "phantase/sensors/%s/message",sensor_id);
      client.publish(msg, "error reading humidity");
      Serial.println("Error reading humidity");
    }
    else {
      display.setTextAlignment(TEXT_ALIGN_LEFT);
      display.drawString(100, 48, "%");
      display.setTextAlignment(TEXT_ALIGN_RIGHT);
      display.drawString(100, 48, String(event.relative_humidity));
      snprintf(msg , 75, "phantase/sensors/%s/humidity",sensor_id);
      int hum1 = (event.relative_humidity - (int)event.relative_humidity)*100;
      snprintf(msg2, 10, "%d.%d",(int)event.relative_humidity,hum1);
      client.publish(msg, msg2);
      Serial.print("Humidity: ");
      Serial.println(event.relative_humidity);
    }
  
    display.display();

  }

}
