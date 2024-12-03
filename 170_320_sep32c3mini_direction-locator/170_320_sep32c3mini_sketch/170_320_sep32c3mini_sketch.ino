// RGB565 color picker https://barth-dev.de/online/rgb565-color-picker/

// Display Adafruit_ST7789 1.9'':
// https://www.aliexpress.com/item/1005006036506903.html?spm=a2g0o.order_list.order_list_main.65.587c1802Q0DfEi#nav-specification

// Arduino esp32c3superMini:
// https://www.aliexpress.com/item/1005005877531694.html?spm=a2g0o.order_list.order_list_main.59.587c1802hvnoYl

// Libs:
// ACAN_ESP32
// Adafruit ST7735 and ST7789 Library

// VCC = +3.3V
// GND = GND
// SCL (SCLK) = GPIO1
// SDA (MOSI) = GPIO0
// RES = RST
// DC  = GPIO3
// CS  = GPIO2
// BLK = +3.3V

// Failed uploading: uploading error: exit status 2
// sudo chmod a+rw /dev/ttyACM0

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

#define TFT_CS         9
#define TFT_RST        4
#define TFT_DC         3
#define TFT_MOSI_SDA   10
#define TFT_SCLK_SCL   6

// Microphone 1
#define MIC0_PIN 0
#define MIC1_PIN 1
#define MIC2_PIN 2

Adafruit_ST7789 TFTscreen = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI_SDA, TFT_SCLK_SCL, TFT_RST);

uint16_t YELLOW = 0xFFE0;
uint16_t GREEN = 0x07E0;
uint16_t RED = 0xF800;
uint16_t BLUE = 0x001F;
uint16_t WHITE = 0xFFFF;
uint16_t BLACK = 0x0000;

int maxY = 170;
int maxX = 320;

// Button
#define BUTTON_PIN 5
#define DEBOUNCE_DELAY 500 // Debounce time in ms
#define MODE_COUNT 3           // Number of modes
int currentMode = 0;       // Current mode
unsigned long lastPress = 0; // Timestamp of last button press

#define SECTION_HEIGHT (maxY / 3)

const int MIC_0_X = 60;
const int MIC_0_Y = 150;
const int MIC_1_X = 160;
const int MIC_1_Y = 20;
const int MIC_2_X = 260;
const int MIC_2_Y = 150;

const int micRealPosition0X = 0;
const int micRealPosition0Y = 0;
const int micRealPosition1X = 20;
const int micRealPosition1Y = 36;
const int micRealPosition2X = 40;
const int micRealPosition2Y = 0;

const int SOUND_THRESHOLD = 1000;

volatile float mic0Time = 0;
volatile float mic1Time = 0;
volatile float mic2Time = 0;

volatile float mic0Value = 0;
volatile float mic1Value = 0;
volatile float mic2Value = 0;

// To redraw only circle
int prevSourceX = -1;
int prevSourceY = -1;


volatile int loopCounter = 0;

/////////////////////////
/// Helpers
/////////////////////////
void drawRound() {
  TFTscreen.fillCircle(maxX/2, maxY/2, maxY/2, YELLOW);
}

void resetScreen() {
  TFTscreen.fillScreen(0x0000);
}

/////////////////////////
/// Draw tests
/////////////////////////
void drawTestWaves() {
  static uint16_t xPos = 0; // Horizontal position on the screen

  // Map the sound value to screen height
  int mic0Y = map(mic0Value, 0, 4095, 0, SECTION_HEIGHT - 1);
  int mic1Y = map(mic1Value, 0, 4095, 0, SECTION_HEIGHT - 1);
  int mic2Y = map(mic2Value, 0, 4095, 0, SECTION_HEIGHT - 1);

  // Draw a vertical line for the current sound value
  TFTscreen.drawLine(xPos, 0, xPos, maxY, BLACK); // Clear previous line

  // // Draw the new line
  TFTscreen.drawLine(xPos, SECTION_HEIGHT, xPos, SECTION_HEIGHT - (mic0Y), RED);
  TFTscreen.drawLine(xPos, SECTION_HEIGHT * 2, xPos, (SECTION_HEIGHT * 2) - (mic1Y), GREEN);
  TFTscreen.drawLine(xPos, SECTION_HEIGHT * 3, xPos, (SECTION_HEIGHT * 3) - (mic2Y), BLUE);

  // Move to the next position
  xPos++;
  if (xPos >= maxX) {
    xPos = 0; // Wrap around
  }
}

void showMicroLog() {
  TFTscreen.setCursor(0, 0);
  TFTscreen.setTextColor(WHITE);
  TFTscreen.print("MIC0: ");
  TFTscreen.println(analogRead(MIC0_PIN));
  TFTscreen.print("MIC1: ");
  TFTscreen.println(analogRead(MIC1_PIN));
  TFTscreen.print("MIC2: ");
  TFTscreen.println(analogRead(MIC2_PIN));

  // clear
  delay(200);

  TFTscreen.fillRect(0, 0, 140, 50, BLACK);
}

void showButtonLog() {
  TFTscreen.setCursor(0, 0);
  TFTscreen.setTextColor(WHITE);
  TFTscreen.print("Button: ");
  TFTscreen.println(digitalRead(BUTTON_PIN));

  // clear
  delay(200);

  TFTscreen.fillRect(0, 0, 120, 20, BLACK);
}





void constrainLineToCircle(int &x, int &y, int centerX, int centerY, int radius) {
  float dx = x - centerX;
  float dy = y - centerY;
  float distance = sqrt(dx * dx + dy * dy);

  if (distance > radius) {
    float scale = radius / distance;
    x = centerX + dx * scale;
    y = centerY + dy * scale;
  }
}

void calculateSoundSource() {
  // Calculate time differences
  float t12 = (mic1Time - mic0Time); // Time difference between mic1 and mic2
  float t13 = (mic2Time - mic0Time); // Time difference between mic1 and mic3

  // Calculate the sound source position using triangulation
  float A = 2 * (micRealPosition1X - micRealPosition0X);
  float B = 2 * (micRealPosition1Y - micRealPosition0Y);
  float C = 2 * (micRealPosition2X - micRealPosition0X);
  float D = 2 * (micRealPosition2Y - micRealPosition0Y);

  float E = t12 * t12 - micRealPosition1X * micRealPosition1X - micRealPosition1Y * micRealPosition1Y + micRealPosition0X * micRealPosition0X + micRealPosition0Y * micRealPosition0Y;
  float F = t13 * t13 - micRealPosition2X * micRealPosition2X - micRealPosition2Y * micRealPosition2Y + micRealPosition0X * micRealPosition0X + micRealPosition0Y * micRealPosition0Y;

  float denominator = A * D - B * C;
  if (denominator == 0) {
    // Avoid division by zero
    return;
  }

  int sourceX = (E * D - B * F) / denominator;
  int sourceY = (A * F - E * C) / denominator;

    // TFTscreen.setCursor(0, 0);
    // TFTscreen.setTextColor(WHITE);
    // TFTscreen.println(t12);
    // TFTscreen.println(t13);
    // TFTscreen.fillRect(0, 0, maxX, 50, BLACK);

  // Constrain values within the screen bounds
  sourceX = constrain(sourceX, 0, maxX);
  sourceY = constrain(sourceY, 0, maxY);

  // Constrain the line to the circle
  constrainLineToCircle(sourceX, sourceY, maxX / 2, maxY / 2, maxY / 2);

  // Erase previous line
  if (prevSourceX >= 0 && prevSourceY >= 0) {
    TFTscreen.drawLine(maxX / 2, maxY / 2, prevSourceX, prevSourceY, BLACK); // Draw over the old line
  }

  // Draw the new line
  TFTscreen.drawLine(maxX / 2, maxY / 2, sourceX, sourceY, RED);

  // Update previous position
  prevSourceX = sourceX;
  prevSourceY = sourceY;
}

void soundDirectionDetector() {
  if (mic0Value > SOUND_THRESHOLD) {
    mic0Time = millis();
  }

  if (mic1Value > SOUND_THRESHOLD) {
    mic1Time = millis();
  }

  if (mic2Value > SOUND_THRESHOLD) {
    mic2Time = millis();
  }

  if (mic0Time > 0 && mic1Time > 0 && mic2Time > 0) {
    // If all timestamps are captured, calculate position
    calculateSoundSource();

    // Reset timestamps for next detection
    mic0Value = mic1Value = mic2Value = 0;
  }
}




/////////////////////////
/// Setup
/////////////////////////
void setup() {
  TFTscreen.init(maxY, maxX);
  TFTscreen.setRotation(1);
  TFTscreen.setTextColor(GREEN);
  resetScreen();
  TFTscreen.setTextSize(2);
  // drawRound();

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // For 0 mode
  TFTscreen.drawCircle(maxX/2, maxY/2, maxY/2, YELLOW);
}

/////////////////////////
/// Loop
/////////////////////////
void loop() {
  // loopCounter++;

  int buttonValue = digitalRead(BUTTON_PIN);

  if (millis() - lastPress > DEBOUNCE_DELAY) {
    if (buttonValue == 1) {
      currentMode++;
      if (currentMode > MODE_COUNT) {
        resetScreen();
        currentMode = 0;

        if (currentMode == 0) {
          TFTscreen.drawCircle(maxX/2, maxY/2, maxY/2, YELLOW);
        }
      }
      lastPress = millis();
    }
  }

  mic0Value = analogRead(MIC0_PIN);
  mic1Value = analogRead(MIC1_PIN);
  mic2Value = analogRead(MIC2_PIN);

  switch (currentMode) {
    case 0:
      soundDirectionDetector();
      break;
    case 1:
      drawTestWaves();
      break;
    case 2:
      showMicroLog();
      break;
    case 3:
      showButtonLog();
      break;
  }
}
