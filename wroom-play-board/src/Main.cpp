/*
 * The board: esp32 s3 wroom 1 n16r8
 *
 * Display: 2.4" SPI TFT 240x320, cd card, no touch
 * Pins:
  SCK (CLK)	GPIO14	SPI Clock
  MOSI	    GPIO13	SPI Data
  CS	      GPIO15	Chip Select display
  DC	      GPIO2	  Data/Command
  RESET	    GPIO4	  Reset
  GND	      GND	    Ground
  VCC	      3.3V
 *
 * Buttons: https://www.instructables.com/How-to-access-5-buttons-through-1-Arduino-input/
 * Pins: GPIO1
 *
 * CD Card pins:
  // SPI спільні:
  MOSI → GPIO 13
  MISO → GPIO 11
  SCK  → GPIO 14

  // CS різні:
  TFT_CS → GPIO 15
  SD_CS  → GPIO 12
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// Define pins for TFT display
#define TFT_CS    15
#define TFT_DC     2
#define TFT_MOSI  13
#define TFT_CLK   14
#define TFT_RESET  4

#define BUTTON_PIN 1

#define HEIGHT  240
#define WIDTH   320

#ifndef LED_BUILTIN
  #define LED_BUILTIN 2
#endif

// Create an instance of the display
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RESET);
// The Adafruit_ILI9341 library auto-detects the display size (240x320) based on the controller.
// If you want to ensure the correct width/height, use tft.width() and tft.height() after tft.begin().
// No need to set size manually for ILI9341; it's fixed at 240x320 pixels.

int lastButton = -1;

void drawTestPattern() {
  // Draw some shapes and text
  tft.fillRect(0, 0, WIDTH/2, HEIGHT/2, ILI9341_RED);
  tft.fillRect(WIDTH/2, 0, WIDTH/2, HEIGHT/2, ILI9341_GREEN);
  tft.fillRect(0, HEIGHT/2, WIDTH/2, HEIGHT/2, ILI9341_BLUE);
  tft.fillRect(WIDTH/2, HEIGHT/2, WIDTH/2, HEIGHT/2, ILI9341_YELLOW);

  // Draw a border
  tft.drawRect(0, 0, tft.width(), tft.height(), ILI9341_WHITE);

  // Add some text
  tft.setCursor(WIDTH/4, HEIGHT/2 - 20);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.println("ESP32 Test");

  tft.setCursor(WIDTH/4 - 10, HEIGHT/2 + 20);
  tft.setTextColor(ILI9341_BLACK);
  tft.println("TFT Display");
}

void displayButtonPress(int button) {
  // Clear the previous button text area
  tft.fillRect(WIDTH/4 - 20, HEIGHT/2 + 50, 120, 30, ILI9341_BLACK);

  // Display the current button press
  tft.setCursor(WIDTH/4 - 10, HEIGHT/2 + 50);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  int adcValue = analogRead(BUTTON_PIN);

  if (button == -1) {
    tft.println(adcValue);
  } else {
    tft.print("Button S");
    tft.println(button);
  }
}

int readButton() {
  int adcValue = analogRead(BUTTON_PIN);
  Serial.print("ADC Value: ");
  Serial.println(adcValue);

  // Map ADC value to button
  if (adcValue > 3700) return 1;    // S1
  if (adcValue > 3300) return 2;    // S2
  if (adcValue > 2500) return 3;    // S3
  if (adcValue > 2200) return 4;    // S4
  if (adcValue > 1850) return 5;    // S5
  if (adcValue > 1650) return 6;    // S6
  return -1;                        // No button pressed
}

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 TFT Display Test");

  tft.begin();
  tft.setRotation(0);

  Serial.println(F("Display initialized"));
  Serial.print(F("Width: ")); Serial.println(tft.width());
  Serial.print(F("Height: ")); Serial.println(tft.height());

  tft.fillScreen(ILI9341_BLACK);

  drawTestPattern();

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  int button = readButton();

  // Update display only if the button state changes
  lastButton = button;
  displayButtonPress(button);

  delay(100); // Small delay to debounce and reduce CPU load
}
