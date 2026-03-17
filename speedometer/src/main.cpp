// ESP32-S3-WROOM1 N16R8 N8R2 44Pin

#include <Arduino.h>
#include <TM1637.h>
#include "webserver.h"
#include "clock.h"

// Servo 1
const int B1_1 = 13; //tmp (48)
const int B2_1 = 47;
const int B3_1 = 21;
const int B4_1 = 11;
// Servo 2
const int B5_1 = 12;
const int B6_1 = 10;
const int B7_1 = 14;
const int B8_1 = 14;
// Servo 3
const int B1_2 = 6;
const int B2_2 = 7;
const int B3_2 = 15;
const int B4_2 = 16;
// Servo 4
const int B5_2 = 17;
const int B6_2 = 8;
const int B7_2 = 3;
const int B8_2 = 9;
// Servo 5
const int B5_3 = 4;
const int B6_3 = 5;
const int B7_3 = 1;
const int B8_3 = 2;
// Display
const int DISPLAY_CLK = 40;
const int DISPLAY_DIO = 41;
// LED
const int LED_PIN_1 = 35;
const int LED_PIN_2 = 36;
const int LED_PIN_3 = 18;
const int LED_PIN_4 = 37;
const int LED_PIN_BACK_LEFT = 39;
const int LED_PIN_BACK_RIGT = 38;

const int servo1[] = {B7_2, B8_2, B5_2, B6_2}; // Physical servo 4 - TEMPORARILY DISABLED
const int servo2[] = {B1_1, B2_1, B3_1, B4_1}; // Physical servo 1
const int servo3[] = {B7_1, B8_1, B5_1, B6_1}; // Physical servo 2
const int servo4[] = {B1_2, B2_2, B3_2, B4_2}; // Physical servo 3
const int servo5[] = {B7_3, B8_3, B5_3, B6_3}; // Physical servo 5

// Display array
const int displayPins[] = {DISPLAY_CLK, DISPLAY_DIO};

// LED array
const int ledPins[] = {
  LED_PIN_1,
  LED_PIN_2,
  LED_PIN_3,
  LED_PIN_4,
  LED_PIN_BACK_LEFT,
  LED_PIN_BACK_RIGT,
};

// TM1637 Display (6 digits)
TM1637 display;

// System stats (0-100%)
int cpuUsage = 0;
int ramUsage = 0;
int batteryLevel = 0;
int networkLoad = 0;
int diskUsage = 0;

// Servo max ranges (in steps)
const int servoRanges[5] = {150, 150, 550, 150, 150};

// Servo positions (0-max range) - logical position for tracking
int servoPositions[5] = {0, 0, 0, 0, 0};

// Servo step sequence counters (for stepper motor coil sequence)
int servoStepCounters[5] = {0, 0, 0, 0, 0};

void step(int s, const int pins[4]) {
  switch(s % 4) {
    case 0: // Котушка А: (+ -), Котушка Б: (off)
      digitalWrite(pins[0], LOW);  digitalWrite(pins[1], HIGH);
      digitalWrite(pins[2], LOW);  digitalWrite(pins[3], LOW);
      break;
    case 1: // Котушка А: (off), Котушка Б: (+ -)
      digitalWrite(pins[0], LOW);  digitalWrite(pins[1], LOW);
      digitalWrite(pins[2], LOW);  digitalWrite(pins[3], HIGH);
      break;
    case 2: // Котушка А: (- +), Котушка Б: (off)
      digitalWrite(pins[0], HIGH); digitalWrite(pins[1], LOW);
      digitalWrite(pins[2], LOW);  digitalWrite(pins[3], LOW);
      break;
    case 3: // Котушка А: (off), Котушка Б: (- +)
      digitalWrite(pins[0], LOW);  digitalWrite(pins[1], LOW);
      digitalWrite(pins[2], HIGH); digitalWrite(pins[3], LOW);
      break;
  }
}

// Forward declaration
void rotateServo(const int pins[4], int steps, bool forward, int servoIndex, bool enforceLimits);

// Rotate servo by specified steps in given direction
// pins: servo pin array [4]
// steps: number of steps to rotate
// forward: true for forward rotation, false for backward
// servoIndex: index (0-4) to track position
void rotateServo(const int pins[4], int steps, bool forward, int servoIndex) {
  rotateServo(pins, steps, forward, servoIndex, true);
}

// Rotate servo with optional limit enforcement
void rotateServo(const int pins[4], int steps, bool forward, int servoIndex, bool enforceLimits) {
  int maxRange = servoRanges[servoIndex];
  
  if (forward) {
    // Forward rotation
    for (int i = 0; i < steps; i++) {
      step(i, pins);

      if (enforceLimits) {
        servoPositions[servoIndex] = (i + 1) % maxRange;
      }
      // Don't update position when bypassing limits - we'll set it manually after reset
      delay(3);
    }
  } else {
    // Backward rotation
    for(int i = steps; i >= 0; i--) {
      step(i, pins);
      
      if (enforceLimits) {
        servoPositions[servoIndex] = (i - 1 + maxRange) % maxRange;
      }
      // Don't update position when bypassing limits - we'll set it manually after reset
      delay(3);
    }
  }
}

// Set servo to absolute position (0-100%)
void setServoPosition(const int pins[4], int percentage, int servoIndex) {
  if (percentage < 0) percentage = 0;
  if (percentage > 100) percentage = 100;
  
  // Convert percentage to steps using custom range for each servo
  int maxRange = servoRanges[servoIndex];
  int targetPosition = (percentage * maxRange) / 100;
  int currentPosition = servoPositions[servoIndex];
  
  // Calculate shortest path
  int diff = targetPosition - currentPosition;
  
  if (diff > 0) {
    rotateServo(pins, diff, true, servoIndex);
  } else if (diff < 0) {
    rotateServo(pins, -diff, false, servoIndex);
  }
}

// Update all servos based on system stats
void updateSystemStats() {
  // setServoPosition(servo1, ramUsage, 0);      // Servo1 (150) - RAM
  // setServoPosition(servo2, batteryLevel, 1);  // Servo2 (150) - Battery
  setServoPosition(servo3, cpuUsage, 2);      // Servo3 (550) - CPU
  // setServoPosition(servo4, networkLoad, 3);   // Servo4 (150) - Network
  // setServoPosition(servo5, diskUsage, 4);     // Servo5 (150) - Disk
}

// Set system stats (called from webserver)
void setSystemStats(int cpu, int ram, int battery, int network, int disk) {
  // Limit values to 0-100% range
  cpuUsage = constrain(cpu, 0, 100);
  ramUsage = constrain(ram, 0, 100);
  batteryLevel = constrain(battery, 0, 100);
  networkLoad = constrain(network, 0, 100);
  diskUsage = constrain(disk, 0, 100);
  
  Serial.println("Stats updated - CPU:" + String(cpuUsage) + "% RAM:" + String(ramUsage) + 
                 "% BAT:" + String(batteryLevel) + "% NET:" + String(networkLoad) + 
                 "% DISK:" + String(diskUsage) + "%");
}

// Reset all servos to position 0
void resetAllServos() {
  const int* servos[] = {servo1, servo2, servo3, servo4, servo5};
  
  // Reset without limits - allow rotation past range to ensure zero position
  rotateServo(servo1, 150, true, 0, false);
  rotateServo(servo1, 400, false, 0, false);

  rotateServo(servo2, 150, true, 1, false);
  rotateServo(servo2, 400, false, 1, false);

  // CPU
  rotateServo(servo3, 550, true, 2, false);
  rotateServo(servo3, 300, false, 2, false);

  rotateServo(servo4, 150, true, 3, false);
  rotateServo(servo4, 400, false, 3, false);

  rotateServo(servo5, 150, true, 4, false);
  rotateServo(servo5, 400, false, 4, false);
  
  // Reset position tracking - all servos are now at physical position 0
  for (int i = 0; i < 5; i++) {
    servoPositions[i] = 0;
    servoStepCounters[i] = 0;
  }
  
  // Turn off all coils to save power
  for (int i = 0; i < 5; i++) {
    digitalWrite(servos[i][0], LOW);
    digitalWrite(servos[i][1], LOW);
    digitalWrite(servos[i][2], LOW);
    digitalWrite(servos[i][3], LOW);
  }
}

// Enable back LEDs
void enableBackLEDs() {
  digitalWrite(LED_PIN_BACK_LEFT, HIGH);
  digitalWrite(LED_PIN_BACK_RIGT, HIGH);
  digitalWrite(LED_BUILTIN, HIGH);  // Test LED
}

void enableFrontLeds() {
  digitalWrite(LED_PIN_1, HIGH);
  digitalWrite(LED_PIN_2, HIGH);
  digitalWrite(LED_PIN_3, HIGH);
  digitalWrite(LED_PIN_4, HIGH);
}

void setup() {
  pinMode(B1_1, OUTPUT); pinMode(B2_1, OUTPUT); pinMode(B3_1, OUTPUT); pinMode(B4_1, OUTPUT);
  pinMode(B5_1, OUTPUT); pinMode(B6_1, OUTPUT); pinMode(B7_1, OUTPUT); pinMode(B8_1, OUTPUT);
  pinMode(B1_2, OUTPUT); pinMode(B2_2, OUTPUT); pinMode(B3_2, OUTPUT); pinMode(B4_2, OUTPUT);
  pinMode(B5_2, OUTPUT); pinMode(B6_2, OUTPUT); pinMode(B7_2, OUTPUT); pinMode(B8_2, OUTPUT);
  pinMode(B5_3, OUTPUT); pinMode(B6_3, OUTPUT); pinMode(B7_3, OUTPUT); pinMode(B8_3, OUTPUT);
  pinMode(LED_PIN_3, OUTPUT); pinMode(LED_PIN_1, OUTPUT); pinMode(LED_PIN_2, OUTPUT);
  pinMode(LED_PIN_4, OUTPUT); pinMode(LED_PIN_BACK_RIGT, OUTPUT); pinMode(LED_PIN_BACK_LEFT, OUTPUT);
  // pinMode(LED_BUILTIN, OUTPUT);
  
  // Initialize 6-digit display
  display.begin(DISPLAY_CLK, DISPLAY_DIO, 6);
  display.setBrightness(7);
  display.setDigitOrder(3, 4, 5, 0, 1, 2);

  // Initialize clock
  initClock(&display);

  enableBackLEDs();
  enableFrontLeds();
  
  // Initialize web server
  initWebServer();
  // resetAllServos();
}

void loop() {
  // Display time on TM1637
  static unsigned long lastDisplayUpdate = 0;
  if (millis() - lastDisplayUpdate >= 500) {  // Update display every 500ms
    displayTime();
    lastDisplayUpdate = millis();
  }
  
  // Update servo positions based on system stats
  static unsigned long lastServoUpdate = 0;
  if (millis() - lastServoUpdate >= 100) {  // Update servos every 100ms
    updateSystemStats();
    lastServoUpdate = millis();
  }
  
  // Handle web server
  handleWebServer();
}