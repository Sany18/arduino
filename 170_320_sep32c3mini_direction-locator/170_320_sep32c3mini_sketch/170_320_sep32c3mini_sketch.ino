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
#define MODE_COUNT 2           // Number of modes
int currentMode = 0;       // Current mode
unsigned long lastPress = 0; // Timestamp of last button press

#define SECTION_HEIGHT (maxY / 3)

// Drow round
void drawRound() {
  TFTscreen.fillCircle(maxX/2, maxY/2, maxY/2, YELLOW);
}

// Reset screen
void resetScreen() {
  TFTscreen.fillScreen(0x0000);
}

void drawTestWaves() {
  static uint16_t xPos = 0; // Horizontal position on the screen

  // Read sound level
  int mic0Value = analogRead(MIC0_PIN);
  int mic1Value = analogRead(MIC1_PIN);
  int mic2Value = analogRead(MIC2_PIN);

  // Map the sound value to screen height
  int mic0Y = map(mic0Value, 0, 4095, 0, SECTION_HEIGHT - 1);
  int mic1Y = map(mic1Value, 0, 4095, 0, SECTION_HEIGHT - 1);
  int mic2Y = map(mic2Value, 0, 4095, 0, SECTION_HEIGHT - 1);

  // Draw a vertical line for the current sound value
  TFTscreen.drawLine(xPos, 0, xPos, maxY, BLACK); // Clear previous line

  // // Draw the new line
  TFTscreen.drawLine(xPos, SECTION_HEIGHT, xPos, SECTION_HEIGHT - (SECTION_HEIGHT - mic0Y), RED);
  TFTscreen.drawLine(xPos, SECTION_HEIGHT * 2, xPos, (SECTION_HEIGHT * 2) - (SECTION_HEIGHT - mic1Y), GREEN);
  TFTscreen.drawLine(xPos, SECTION_HEIGHT * 3, xPos, (SECTION_HEIGHT * 3) - (SECTION_HEIGHT - mic2Y), BLUE);

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
}

/////////////////////////
/// Loop
/////////////////////////
void loop() {
  int buttonValue = digitalRead(BUTTON_PIN);

  if (millis() - lastPress > DEBOUNCE_DELAY) {
    if (buttonValue == 1) {
      currentMode++;
      if (currentMode > MODE_COUNT) {
        currentMode = 0;
      }
      lastPress = millis();
    }
  }

  switch (currentMode) {
    case 0:
      drawTestWaves();
      break;
    case 1:
      showMicroLog();
      break;
    case 2:
      showButtonLog();
      break;
  }
}
