#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// WiFi Credentials
#define WLAN_SSID "SSID" // Replace with your WiFi SSID
#define WLAN_PASS "PASS" // Replace with your WiFi password

// Adafruit IO Setup
#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883 // Standard MQTT port
#define AIO_USERNAME "phthinhnguyen" // Replace with your Adafruit username
#define AIO_KEY "key" // Replace with your Adafruit IO key

// MQTT Client and Publish Setup
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish temp_pub = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/cambien1");

// Global Variables
char temp_buffer[20];             // Buffer for receiving temperature data
unsigned long last_temp_publish = 0; // Timer for 30-second interval
float last_temp_value = 0.0;      // Last temperature value

// Function to connect to WiFi
void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi connected!");
}

// Function to connect to MQTT broker
void connectToMQTT() {
  Serial.println("Connecting to MQTT...");
  while (mqtt.connect() != 0) {
    Serial.println("MQTT connection failed! Retrying...");
    delay(500);
  }
  Serial.println("MQTT connected!");
}

// Function to handle publishing temperature to Adafruit IO
void publishTemperatureToAdafruit(float value) {
  Serial.printf("Publishing temperature: %.2f°C to Adafruit...\n", value);
  if (temp_pub.publish(value)) {
    Serial.println("Successfully published temperature!");
  } else {
    Serial.println("Failed to publish temperature!");
  }
}

// Function to process UART input from STM32
void processSTM32Commands() {
  if (Serial.available()) {
    String received = Serial.readStringUntil('#'); // Read until '#' delimiter
    Serial.print("Received from STM32: ");
    Serial.println(received);

    // Check for temperature data
    if (received.startsWith("!TEMP:")) {
      float temperature = received.substring(6).toFloat(); // Extract temperature value
      last_temp_value = temperature;
      Serial.printf("Received temperature: %.2f°C\n", temperature);
    } else if (received == "o") {
      Serial.println("Heartbeat received from STM32.");
    } else {
      Serial.println("Unknown command received.");
    }
  }
}

// Function to publish temperature to Adafruit IO every 30 seconds
void publishTemperature() {
  unsigned long current_time = millis();
  if (current_time - last_temp_publish >= 30000) { // Check 30-second interval
    last_temp_publish = current_time;
    Serial.println("Publishing temperature to Adafruit...");
    publishTemperatureToAdafruit(last_temp_value);
  }
}

// Function to keep MQTT connection alive
void maintainMQTTConnection() {
  if (!mqtt.connected()) {
    Serial.println("MQTT disconnected. Reconnecting...");
    connectToMQTT();
  }
  mqtt.processPackets(10);
  mqtt.ping();
}

// Setup Function
void setup() {
  // Configure Serial Monitor
  Serial.begin(115200);
  Serial.println("Starting...");

  // Connect to WiFi
  connectToWiFi();

  // Connect to MQTT broker
  connectToMQTT();
}

// Main Loop
void loop() {
  processSTM32Commands(); // Handle commands from STM32
  publishTemperature();   // Publish temperature every 30 seconds
  maintainMQTTConnection(); // Keep MQTT connection alive
}