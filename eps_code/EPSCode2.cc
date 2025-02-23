#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#define LDR_PIN 34 // GPIO for LDR module (analog input)

// Function Prototypes
void readSensors();
void analyzeSleep();
void detectBedEntry();
void calculateSleepDurations();
void analyzeCircadianRhythm();
void calculateSleepScore();
void detectFever();
void generateRecommendations();

// Sensor Setup
#define ONE_WIRE_BUS 4 // GPIO for DS18B20 temperature sensors
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensors(&oneWire);
Adafruit_MPU6050 mpu;
// Removed BH1750 instance, using LDR module on analog pin

// Data Variables
float cumulativeLight = 0;
float cumulativeTemp = 0;
float cumulativeMovement = 0;
int readingCount = 0;

float avgLight = 0;
float avgTemperature = 0;
float avgMovement = 0;
float circadianRhythmScore = 100; // Score to predict circadian misalignment (bounded between 0-100)
float sleepScore = 100;

// User Inputs
int userAge = 25;
String desiredSleepSchedule = "22:00-06:00";
String userDiet = "Balanced";

// Sleep Stage Variables
int sleepStage = 0; // 0: Awake, 1: Light Sleep, 2: REM, 3: Deep Sleep
float sleepQualityScore = 0;
bool inBed = false;
unsigned long bedEntryTime = 0;
unsigned long sleepOnsetTime = 0;
unsigned long bedExitTime = 0;
float hoursInBed = 0;
float hoursSlept = 0;
float timeAwake = 0;
float timeLightSleep = 0;
float timeREM = 0;
float timeDeepSleep = 0;

void setup() {
    Serial.begin(115200);
    Wire.begin();

    // Initialize Sensors
    tempSensors.begin();
    // Removed BH1750 initialization, using LDR module
    mpu.begin();
}

void loop() {
    readSensors();
    analyzeSleep();
    detectBedEntry();
    calculateSleepDurations();
    analyzeCircadianRhythm();
    calculateSleepScore();
    detectFever();
    generateRecommendations();
    delay(5000); // Update every 5 seconds
}

void readSensors() {
    float lightReading = analogRead(LDR_PIN); // Read LDR sensor
    float tempReading;
    float movement;

    tempSensors.requestTemperatures();
    tempReading = tempSensors.getTempCByIndex(0);

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    movement = abs(a.acceleration.x) + abs(a.acceleration.y) + abs(a.acceleration.z);

    // Accumulate values
    cumulativeLight += lightReading;
    cumulativeTemp += tempReading;
    cumulativeMovement += movement;
    readingCount++;

    // Calculate averages
    avgLight = cumulativeLight / readingCount;
    avgTemperature = cumulativeTemp / readingCount;
    avgMovement = cumulativeMovement / readingCount;

    Serial.print("Light Level: "); Serial.println(avgLight);
    Serial.print("Acceleration Sum: "); Serial.println(avgMovement);
    Serial.print("Body Temperature: "); Serial.println(avgTemperature);
}

void analyzeSleep() {
    if (avgMovement > 1.5) {
        sleepStage = 0; // Awake
        timeAwake += 5.0 / 3600;
    } else if (avgMovement > 0.5 && avgMovement <= 1.5) {
        sleepStage = 1; // Light Sleep
        timeLightSleep += 5.0 / 3600;
    } else if (avgMovement <= 0.5 && avgTemperature >= 36.0 && avgTemperature <= 36.5) {
        sleepStage = 2; // REM Sleep
        timeREM += 5.0 / 3600;
    } else if (avgMovement <= 0.2 && avgTemperature < 36.0) {
        sleepStage = 3; // Deep Sleep
        timeDeepSleep += 5.0 / 3600;
    }
    Serial.print("Current Sleep Stage: "); Serial.println(sleepStage);
}

void detectBedEntry() {
    if (!inBed && avgMovement < 0.5) {
        inBed = true;
        bedEntryTime = millis();
        Serial.println("User entered bed.");
    } else if (inBed && avgMovement > 1.5) {
        inBed = false;
        bedExitTime = millis();
        Serial.println("User exited bed.");
    }
}

void calculateSleepDurations() {
    if (inBed) {
        hoursInBed = (millis() - bedEntryTime) / 3600000.0;
        hoursSlept = timeLightSleep + timeREM + timeDeepSleep;
    }
    Serial.print("Hours in Bed: "); Serial.println(hoursInBed);
    Serial.print("Hours Slept: "); Serial.println(hoursSlept);
}

void analyzeCircadianRhythm() {
    circadianRhythmScore = (circadianRhythmScore + avgLight / 2) / 2;
    circadianRhythmScore = constrain(circadianRhythmScore, 0, 100);
    Serial.print("Circadian Rhythm Score: "); Serial.println(circadianRhythmScore);
}

void calculateSleepScore() {
    sleepScore = (hoursSlept / 8.0) * 50 + (timeDeepSleep / hoursSlept) * 25 + (timeREM / hoursSlept) * 15 + (timeLightSleep / hoursSlept) * 10;
    sleepScore = constrain(sleepScore, 0, 100);
    Serial.print("Sleep Score: "); Serial.println(sleepScore);
}

void detectFever() {
    if (avgTemperature >= 38.0) {
        Serial.println("⚠️ High Temperature Detected! Possible Fever.");
    } else if (avgTemperature >= 37.5 && avgTemperature < 38.0) {
        Serial.println("⚠️ Elevated Temperature: Monitor for potential fever symptoms.");
    } else if (avgTemperature >= 36.5 && avgTemperature < 37.5) {
        Serial.println("✅ Normal Body Temperature.");
    } else if (avgTemperature < 36.5) {
        Serial.println("🔹 Slightly Lower Temperature: May be due to sleep cycle variations.");
    }
}

void generateRecommendations() {
    Serial.println("Recommendations:");
    if (sleepScore < 60) {
        Serial.println("- Improve sleep consistency. Try to sleep and wake at the same time daily.");
    }
    if (timeDeepSleep < 1.5) {
        Serial.println("- Increase deep sleep by avoiding screens before bed and reducing caffeine.");
    }
    if (circadianRhythmScore < 50) {
        Serial.println("- Your circadian rhythm is misaligned. Try getting more morning light exposure.");
    }
    if (avgLight > 200) {
        Serial.println("- Reduce bright light exposure before bedtime to improve melatonin production.");
    }
    Serial.print("Final Circadian Rhythm Score: "); Serial.println(circadianRhythmScore);
    Serial.print("Final Sleep Score: "); Serial.println(sleepScore);
}
