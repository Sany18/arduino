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

// Rotate servo by specified steps in given direction
// pins: servo pin array [4]
// steps: number of steps to rotate
// forward: true for forward rotation, false for backward
void rotateServo(const int pins[4], int steps, bool forward) {
  static int position = 0;  // Track current position
  
  if (forward) {
    // Forward rotation
    for (int i = 0; i < steps; i++) {
      step(position, pins);
      position = (position + 1) % 400;  // Keep position in range 0-399
      delay(3);
    }
  } else {
    // Backward rotation
    for (int i = 0; i < steps; i++) {
      step(position, pins);
      position = (position - 1 + 400) % 400;  // Keep position in range 0-399
      delay(3);
    }
  }
}

// Reset all servos to position 0
void resetAllServos() {
  const int* servos[] = {servo1, servo2, servo3, servo4, servo5};
  
  rotateServo(servo1, 150, true);
  rotateServo(servo1, 200, false);

  rotateServo(servo2, 150, true);
  rotateServo(servo2, 200, false);

  rotateServo(servo3, 550, true);
  rotateServo(servo3, 300, false);

  rotateServo(servo4, 150, true);
  rotateServo(servo4, 200, false);

  rotateServo(servo5, 150, true);
  rotateServo(servo5, 200, false);
  
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
  resetAllServos();
}

void loop() {
  // Display time on TM1637
  static unsigned long lastDisplayUpdate = 0;
  if (millis() - lastDisplayUpdate >= 500) {  // Update display every 500ms
    displayTime();
    lastDisplayUpdate = millis();
  }
  
  // Handle web server
  handleWebServer();
}