
#include <ESP32Servo.h>
#include <Arduino.h>
// Left and right tail feather servos
Servo leftTailServo;
Servo rightTailServo;
const int leftTailPin = 1;
const int rightTailPin = 2;

// Define the pin for the ELRS receiver CRSF signal
#define CRSF_RX_PIN 3

// Use Serial0 for USB, Serial1 for CRSF
#define CRSF_SERIAL Serial1

void setup() {
  leftTailServo.setPeriodHertz(50); // Standard 50Hz servo
  rightTailServo.setPeriodHertz(50);
  leftTailServo.attach(leftTailPin);
  rightTailServo.attach(rightTailPin);
  CRSF_SERIAL.begin(420000, SERIAL_8N1, CRSF_RX_PIN, -1); // 420k baud, RX only
}

// CRSF protocol constants
#define CRSF_MAX_FRAME_SIZE 64
#define CRSF_ADDRESS_MODULE 0xC8
#define CRSF_ADDRESS_RADIO  0xEA
#define CRSF_FRAMETYPE_RC_CHANNELS_PACKED 0x16

uint8_t crsfBuffer[CRSF_MAX_FRAME_SIZE];
uint8_t crsfIndex = 0;
uint8_t crsfFrameLen = 0;
bool inFrame = false;

void printChannels(const uint8_t* payload) {
  // 11 channels, 16 bits each, packed into 22 bytes (little endian)
  uint16_t channels[16];
  channels[0]  = (payload[0]  | ((payload[1] & 0x07) << 8));
  channels[1]  = ((payload[1] >> 3) | (payload[2] << 5)) & 0x7FF;
  channels[2]  = ((payload[2] >> 6) | (payload[3] << 2) | ((payload[4] & 0x01) << 10)) & 0x7FF;
  channels[3]  = ((payload[4] >> 1) | (payload[5] << 7)) & 0x7FF;
  channels[4]  = ((payload[5] >> 4) | (payload[6] << 4)) & 0x7FF;
  channels[5]  = ((payload[6] >> 7) | (payload[7] << 1) | ((payload[8] & 0x03) << 9)) & 0x7FF;
  channels[6]  = ((payload[8] >> 2) | (payload[9] << 6)) & 0x7FF;
  channels[7]  = ((payload[9] >> 5) | (payload[10] << 3)) & 0x7FF;
  // Channel 0: up/down (elevator), Channel 1: left/right (rudder/aileron)

  // Invert both directions
  int chUpDown = -map(constrain(channels[0], 172, 1811), 172, 1811, -90, 90);   // Centered at 0, inverted
  int chLeftRight = -map(constrain(channels[1], 172, 1811), 172, 1811, -90, 90); // Centered at 0, inverted

  // Mix: both up/down for elevator, opposite for aileron/rudder
  int leftAngle = 90 + chUpDown + chLeftRight;   // 90 is center
  int rightAngle = 90 + chUpDown - chLeftRight;

  // Constrain to servo range
  leftAngle = constrain(leftAngle, 0, 180);
  rightAngle = constrain(rightAngle, 0, 180);

  leftTailServo.write(leftAngle);
  rightTailServo.write(rightAngle);

  Serial.printf("Channels: U/D=%d L/R=%d | LeftTail=%d RightTail=%d\n",
                channels[0], channels[1], leftAngle, rightAngle);
}

void loop() {
  // Read bytes from CRSF
  while (CRSF_SERIAL.available()) {
    uint8_t b = CRSF_SERIAL.read();
    if (!inFrame) {
      // Look for start of frame (address byte)
      if (b == CRSF_ADDRESS_MODULE || b == CRSF_ADDRESS_RADIO) {
        crsfBuffer[0] = b;
        crsfIndex = 1;
        inFrame = true;
      }
    } else {
      crsfBuffer[crsfIndex++] = b;
      if (crsfIndex == 2) {
        crsfFrameLen = b;
        if (crsfFrameLen > CRSF_MAX_FRAME_SIZE - 2) {
          // Invalid length, reset
          inFrame = false;
          crsfIndex = 0;
        }
      } else if (crsfIndex == crsfFrameLen + 2) {
        // Full frame received
        uint8_t type = crsfBuffer[2];
        // If RC channels frame, decode
        if (type == CRSF_FRAMETYPE_RC_CHANNELS_PACKED) {
          printChannels(&crsfBuffer[3]);
        }
        inFrame = false;
        crsfIndex = 0;
      }
    }
  }
}
