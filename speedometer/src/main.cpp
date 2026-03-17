// ESP32-S3-WROOM1 N16R8 N8R2 44Pin

#include <Arduino.h>
#include <TM1637.h>

// Servo 1
const int B1_1 = 48;
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

const int servo1[] = {B7_2, B8_2, B5_2, B6_2}; // Physical servo 4
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

// Reset all servos to position 0
void resetAllServos() {
  const int* servos[] = {servo1, servo2, servo3, servo4, servo5};
  
  for (int i = 0; i < 5; i++) {
    for(int j = 399; j >= 0; j--) {
      step(j, servos[i]);
      delay(3);
    }
    delay(200);
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
  
  // Initialize 6-digit display
  display.begin(DISPLAY_CLK, DISPLAY_DIO, 6);
  display.setBrightness(7);

  // resetAllServos();
  enableBackLEDs();
  enableFrontLeds();
}

// Test function for a single motor
void testMotor(const int pins[4], int motorNum) {
  // Display motor number
  display.displayClear();
  display.displayInt(motorNum);
  
  // Forward rotation
  for(int i = 0; i < 400; i++) {
    step(i, pins);
    delay(3);
  }
  delay(150);
  
  // Backward rotation
  for(int i = 399; i >= 0; i--) {
    step(i, pins);
    delay(3);
  }
  delay(150);
}

// void loop() {
//   // Test all 6 digit positions
//   display.displayClear();
//   delay(500);
  
//   // Count 0-9 on all 6 digits
//   for (int i = 0; i <= 9; i++) {
//     display.displayClear();
//     display.displayInt(i);
//     delay(800);
//   }
  
//   delay(500);
  
//   // Show 123456 (all 6 digits)
//   display.displayClear();
//   display.displayInt(123456);
//   delay(2000);
  
//   // Show with decimal points at different positions
//   for (int pos = 0; pos < 6; pos++) {
//     uint8_t raw[6] = {1, 2, 3, 4, 5, 6};
//     display.displayRaw(raw, pos);  // Decimal point at position
//     delay(800);
//   }
  
//   delay(500);
  
//   // Show 888888 (all segments lit)
//   display.displayClear();
//   display.displayInt(888888);
//   delay(2000);
  
//   // Show time format: 12:34:56
//   display.displayClear();
//   uint8_t raw[6] = {1, 2, 3, 4, 5, 6};
//   // Add dots after positions 1 and 3 (creates 12:34:56)
//   raw[1] |= 0x80;  // Dot after 2
//   raw[3] |= 0x80;  // Dot after 4
//   display.displayRaw(raw, -1);
//   delay(3000);
  
//   // Clear before LED test
//   display.displayClear();
  
//   // Test LEDs sequentially
//   for (int i = 0; i < 6; i++) {
//     digitalWrite(ledPins[i], HIGH);
//     delay(500);
//     digitalWrite(ledPins[i], LOW);
//     delay(500);
//   }
  
//   delay(1000);
// }

void loop() {
  // resetAllServos();
}