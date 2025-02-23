#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define LDR_PIN 34              // Analog input for LDR module
#define ONE_WIRE_BUS 4          // GPIO for DS18B20 temperature sensor

// Wi-Fi credentials
const char* ssid = "your_SSID"; 
const char* password = "your_PASSWORD";

const char* serverUrl = "http://your_server_ip:3000/api/upload"; 

void readSensors();
void sendSensorData();

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensors(&oneWire);
Adafruit_MPU6050 mpu;

float cumulativeLight = 0;
float cumulativeTemp = 0;
float cumulativeMovement = 0;
int readingCount = 0;

float avgLight = 0;
float avgTemperature = 0;
float avgMovement = 0;
float circadianRhythmScore = 100;
float sleepScore = 100;

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("System Starting...");

//change wifi credentials based off current wifi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi.");

  Wire.begin();

  Serial.println("Initializing DS18B20 temperature sensor...");
  tempSensors.begin();
  Serial.println("DS18B20 initialization complete.");

  Serial.println("Initializing MPU6050 sensor...");
  if (!mpu.begin()) {
    Serial.println("MPU6050 initialization failed! Check wiring.");
    while (1) delay(10);
  }
  Serial.println("MPU6050 initialization successful.");
  
  Serial.println("Setup complete.\n");
}

void loop() {
  readSensors(); 
  sendSensorData();

  delay(5000);
}

void readSensors() {
  Serial.println("=== Reading Sensors ===");
  
  int lightReading = analogRead(LDR_PIN);
  
  tempSensors.requestTemperatures();
  float tempReading = tempSensors.getTempCByIndex(0);
  if (tempReading == DEVICE_DISCONNECTED_C) {
    Serial.println("Error: DS18B20 sensor not detected!");
    tempReading = 0;
  }
  
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  float movement = fabs(a.acceleration.x) + fabs(a.acceleration.y) + fabs(a.acceleration.z);
  
  cumulativeLight += lightReading;
  cumulativeTemp += tempReading;
  cumulativeMovement += movement;
  readingCount++;
  
  avgLight = cumulativeLight / readingCount;
  avgTemperature = cumulativeTemp / readingCount;
  avgMovement = cumulativeMovement / readingCount;
  
  // Print sensor values
  Serial.print("Light Reading: "); Serial.print(lightReading);
  Serial.print(" | Average Light: "); Serial.println(avgLight);
  Serial.print("Temperature Reading (°C): "); Serial.print(tempReading);
  Serial.print(" | Average Temperature (°C): "); Serial.println(avgTemperature);
  Serial.print("Movement (acceleration sum): "); Serial.print(movement);
  Serial.print(" | Average Movement: "); Serial.println(avgMovement);
  Serial.println("========================\n");
}

void sendSensorData() {
  String payload = "{\"lightIntensity\": " + String(avgLight) + 
                   ", \"temperatureF\": " + String(avgTemperature * 9 / 5 + 32) + 
                   ", \"sleepScore\": " + String(sleepScore) + 
                   ", \"circadianRhythmScore\": " + String(circadianRhythmScore) + 
                   ", \"movement\": " + String(avgMovement) + "}";

  if(WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl); 
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(payload);
    if (httpResponseCode > 0) {
      Serial.print("Data sent successfully! Response code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error in sending data. HTTP response code: ");
      Serial.println(httpResponseCode);
    }
    http.end(); // Free resources
  } else {
    Serial.println("Error: Not connected to Wi-Fi.");
  }
}
