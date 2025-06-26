#define BLYNK_TEMPLATE_ID "TMPL37sZd6ZUF"
#define BLYNK_TEMPLATE_NAME "Smart Urban Streetlight"

#include <SoftwareSerial.h>
#include <BlynkSimpleStream.h>

// Define pins
#define LDR_PIN A0
#define SOUND_SENSOR A1
#define VIBRATION_SENSOR A2
#define IR1 2
#define IR2 3
#define IR3 4
#define IR4 5
#define LED1 6
#define LED2 7
#define LED3 8
#define LED4 9
#define RED_LED 10
#define BUZZER 11

// Blynk authentication
char auth[] = "YourAuthToken"; // Replace with your Blynk auth token

// Thresholds
const int LDR_THRESHOLD = 500; // Adjust based on your LDR calibration
const int SOUND_THRESHOLD = 60; // Adjust based on your sound sensor calibration
const int VIBRATION_THRESHOLD = 50; // Adjust based on your vibration sensor calibration

// Software Serial for communication with ESP8266
SoftwareSerial espSerial(9, 10); // RX, TX (connected to ESP8266 via level shifter)

// Variables
bool accidentDetected = false;
unsigned long lastAccidentTime = 0;
const unsigned long accidentTimeout = 30000; // 30 seconds

void setup() {
  // Initialize pins
  pinMode(LDR_PIN, INPUT);
  pinMode(SOUND_SENSOR, INPUT);
  pinMode(VIBRATION_SENSOR, INPUT);
  pinMode(IR1, INPUT_PULLUP);
  pinMode(IR2, INPUT_PULLUP);
  pinMode(IR3, INPUT_PULLUP);
  pinMode(IR4, INPUT_PULLUP);
  
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  // Initialize serial communication
  Serial.begin(9600);
  espSerial.begin(9600);
  
  // Initialize Blynk
Blynk.begin(espSerial, auth);  // Correct parameter order  
  // Turn off all LEDs initially
  allLedsOff();
}

void loop() {
  Blynk.run();
  
  // Read sensor values
  int ldrValue = analogRead(LDR_PIN);
  int soundValue = analogRead(SOUND_SENSOR);
  int vibrationValue = analogRead(VIBRATION_SENSOR);

  // Day/Night detection
  if (ldrValue > LDR_THRESHOLD) { // Daytime
    allLedsOff();
  } 
  else { // Nighttime
    // Set dim lighting
    analogWrite(LED1, 50);
    analogWrite(LED2, 50);
    analogWrite(LED3, 50);
    analogWrite(LED4, 50);

    // Check for motion detection (IR sensors are active LOW)
    if (digitalRead(IR1) == LOW) analogWrite(LED1, 255);
    if (digitalRead(IR2) == LOW) analogWrite(LED2, 255);
    if (digitalRead(IR3) == LOW) analogWrite(LED3, 255);
    if (digitalRead(IR4) == LOW) analogWrite(LED4, 255);
  }

  // Accident detection (sound + vibration)
  if (!accidentDetected && 
      soundValue > SOUND_THRESHOLD && 
      vibrationValue > VIBRATION_THRESHOLD) {
    triggerAccident();
  }

  // Reset accident detection after timeout
  if (accidentDetected && (millis() - lastAccidentTime > accidentTimeout)) {
    resetAccident();
  }

  // Check for SOS messages from ESP8266
  if (espSerial.available()) {
    String message = espSerial.readStringUntil('\n');
    message.trim();
    
    if (message == "SOS_TRIGGERED") {
      triggerSOS();
    }
    else if (message == "RESET") {
      resetSystem();
    }
  }

  delay(100);
}

// Helper functions
void allLedsOff() {
  analogWrite(LED1, 0);
  analogWrite(LED2, 0);
  analogWrite(LED3, 0);
  analogWrite(LED4, 0);
  digitalWrite(RED_LED, LOW);
  digitalWrite(BUZZER, LOW);
}

void triggerAccident() {
  accidentDetected = true;
  lastAccidentTime = millis();
  digitalWrite(RED_LED, HIGH);
  digitalWrite(BUZZER, HIGH);
 //arduinoSerial.println("⚠️ Accident Detected!");
  Blynk.logEvent("accident_alert", "Emergency: ⚠️ Accident Detected!");
}

void resetAccident() {
  accidentDetected = false;
  digitalWrite(RED_LED, LOW);
  digitalWrite(BUZZER, LOW);
}

void triggerSOS() {
  digitalWrite(RED_LED, HIGH);
  digitalWrite(BUZZER, HIGH);
  //arduinoSerial.println("🚨 SOS Button Pressed!");
  Blynk.logEvent("sos_alert", "Emergency: 🚨 SOS Button Pressed!");
  delay(10000); // 10 second alarm
  digitalWrite(BUZZER, LOW);
}

void resetSystem() {
  digitalWrite(RED_LED, LOW);
  digitalWrite(BUZZER, LOW);
}

// Blynk virtual pin handlers
BLYNK_WRITE(V0) { digitalWrite(LED1, param.asInt()); }
BLYNK_WRITE(V1) { digitalWrite(LED2, param.asInt()); }
BLYNK_WRITE(V2) { digitalWrite(LED3, param.asInt()); }
BLYNK_WRITE(V3) { digitalWrite(LED4, param.asInt()); }
BLYNK_WRITE(V4) { 
  if (param.asInt()) {
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);
    digitalWrite(LED3, HIGH);
    digitalWrite(LED4, HIGH);
  } else {
    allLedsOff();
  }
}
BLYNK_WRITE(V5) { 
  resetSystem(); 
}