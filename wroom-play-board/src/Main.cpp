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
#include <stddef.h>
#include <WiFi.h>
#include "env.h"

// --- Subclass for custom ILI9341 overrides ---
#define MADCTL_MX  0x40
#define MADCTL_MY  0x80
#define MADCTL_MV  0x20
#define MADCTL_BGR 0x08
#define ILI9341_TFTWIDTH  240
#define ILI9341_TFTHEIGHT 320
#define ILI9341_MADCTL 0x36
#define ILI9341_CASET  0x2A
#define ILI9341_PASET  0x2B
#define ILI9341_RAMWR  0x2C

extern void SPI_WRITE16(uint16_t d);
extern void writeCommand(uint8_t cmd);
extern void sendCommand(uint8_t cmd, const uint8_t *data, uint8_t num);

class MyILI9341 : public Adafruit_ILI9341 {
public:
  using Adafruit_ILI9341::Adafruit_ILI9341;
  void setRotation(uint8_t m) override {
    rotation = m % 4;
    uint8_t madctl = 0;
    switch (rotation) {
      case 0:
        madctl = (MADCTL_MV | MADCTL_MY | MADCTL_BGR);
        _width  = ILI9341_TFTHEIGHT;
        _height = ILI9341_TFTWIDTH;
        break;
      case 1:
        madctl = (MADCTL_MX | MADCTL_MY | MADCTL_BGR);
        _width  = ILI9341_TFTWIDTH;
        _height = ILI9341_TFTHEIGHT;
        break;
      case 2:
        madctl = (MADCTL_MV | MADCTL_MX | MADCTL_BGR);
        _width  = ILI9341_TFTHEIGHT;
        _height = ILI9341_TFTWIDTH;
        break;
      case 3:
        madctl = (MADCTL_BGR);
        _width  = ILI9341_TFTWIDTH;
        _height = ILI9341_TFTHEIGHT;
        break;
    }
    sendCommand(ILI9341_MADCTL, &madctl, 1);
  }
  void setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) override {
    // x=0,y=0 is top-left, x increases downward, y increases rightward
    const uint16_t colstart = 0;
    const uint16_t rowstart = 0;
    uint16_t x0 = x + colstart;
    uint16_t x1 = x + w - 1 + colstart;
    uint16_t y0 = y + rowstart;
    uint16_t y1 = y + h - 1 + rowstart;
    writeCommand(ILI9341_CASET); // Column addr set
    SPI_WRITE16(y0);
    SPI_WRITE16(y1);
    writeCommand(ILI9341_PASET); // Page addr set
    SPI_WRITE16(x0);
    SPI_WRITE16(x1);
    writeCommand(ILI9341_RAMWR); // Write to RAM
  }
};

// Define pins for TFT display
#define TFT_CS    15
#define TFT_DC     2
#define TFT_MOSI  13
#define TFT_CLK   14
#define TFT_RESET  4

#define BUTTON_PIN 1

// Use runtime screen dimensions provided by the driver after init/rotation
int SCREEN_WIDTH = 0;
int SCREEN_HEIGHT = 0;

// #ifndef LED_BUILTIN
//   #define LED_BUILTIN 2
// #endif

MyILI9341 tft = MyILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RESET);

int lastButton = -1;
int lastWiFiStatus = -1;

// Default text size 5x7+1 (space)

void displayWiFiStatus() {
  // Clear WiFi status area (below button info)
  tft.fillRect(0, 15, SCREEN_WIDTH, 30, ILI9341_BLACK);
  tft.setCursor(0, 15);
  tft.setTextSize(2);
  if (WiFi.status() == WL_CONNECTED) {
    tft.setTextColor(ILI9341_GREEN);
    tft.print("WiFi: ");
    tft.println(WiFi.SSID());
    tft.setTextColor(ILI9341_WHITE);
    tft.print("IP: ");
    tft.println(WiFi.localIP());
  } else if (WiFi.status() == WL_DISCONNECTED) {
    tft.setTextColor(ILI9341_RED);
    tft.println("WiFi: Disconnected");
  } else {
    tft.setTextColor(ILI9341_YELLOW);
    tft.println("Connecting...");
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
  lastWiFiStatus = WiFi.status();
  Serial.begin(115200);
  Serial.println("ESP32 TFT Display Test");

  // WiFi credentials from env.h
  const char* ssid = WIFI_SSID;
  const char* password = WIFI_PASSWORD;

  tft.begin();
  tft.setRotation(2);
  SCREEN_WIDTH = tft.width();
  SCREEN_HEIGHT = tft.height();
  tft.fillScreen(ILI9341_BLACK);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // Start WiFi connection
  WiFi.begin(ssid, password);
  int wifiTries = 0;
  while (WiFi.status() != WL_CONNECTED && wifiTries < 40) { // ~20s timeout
    displayWiFiStatus();
    delay(500);
    wifiTries++;
  }
  displayWiFiStatus(); // Show final status
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi connection failed.");
  }
}

void loop() {
  int button = readButton();
  lastButton = button;

  int currentWiFiStatus = WiFi.status();
  static unsigned long lastReconnectAttempt = 0;
  if (currentWiFiStatus != lastWiFiStatus) {
    displayWiFiStatus();
    lastWiFiStatus = currentWiFiStatus;
  }

  // If disconnected, try to reconnect every 2 seconds
  if (currentWiFiStatus != WL_CONNECTED) {
    unsigned long now = millis();
    if (now - lastReconnectAttempt > 2000) {
      WiFi.reconnect();
      lastReconnectAttempt = now;
    }
  }

  delay(100); // Small delay to debounce and reduce CPU load
}
