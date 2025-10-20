// Required by Arduino framework, even if unused
void loop() {}

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <stddef.h>
#include <WiFi.h>
#include <Ticker.h>
#include "env.h"
#include "menu.h"
#include "MyILI9341.h"
#include "display_helpers.h"

// Define pins for TFT display
#define TFT_CS    15
#define TFT_DC     2
#define TFT_MOSI  13
#define TFT_CLK   14
#define TFT_RESET  4

#define BUTTON_PIN 1

// Globals
int SCREEN_WIDTH = 0;
int SCREEN_HEIGHT = 0;
MyILI9341 tft = MyILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RESET);
int lastButton = -1;
int lastWiFiStatus = -1;
Ticker buttonTicker;
Ticker wifiReconnectTicker;

// Function declarations
void displayWiFiStatus();
int readButton();
void pollButton();
void reconnectWiFi();
void onWiFiEvent(WiFiEvent_t event);

void setup() {
  // Initialize menu system
  menuInit(&tft);
  Serial.begin(115200);
  Serial.println("ESP32 TFT Display Test");

  tft.begin();
  tft.setRotation(2);
  SCREEN_WIDTH = tft.width();
  SCREEN_HEIGHT = tft.height();
  tft.fillScreen(ILI9341_BLACK);
  menuRender();

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); // LED off by default

  // Register WiFi event handler
  WiFi.onEvent(onWiFiEvent);

  // Start button polling every 100ms
  buttonTicker.attach_ms(100, pollButton);

  // Start WiFi reconnect timer every 2s
  wifiReconnectTicker.attach(2, reconnectWiFi);

  lastWiFiStatus = WiFi.status();
}

// WiFi event handler
void onWiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case SYSTEM_EVENT_STA_CONNECTED:
    case SYSTEM_EVENT_STA_GOT_IP:
    case SYSTEM_EVENT_STA_DISCONNECTED:
      displayWiFiStatus();
      break;
    default:
      break;
  }
}

// Poll button state
void pollButton() {
  int button = readButton();
  static bool lastWiFiState = false;
  static bool lastLedState = false;
  if (button != lastButton) {
    lastButton = button;
    // S4 = up, S5 = down, S6 = select, S1 = back
    if (button == 4) {
      menuUp();
    } else if (button == 5) {
      menuDown();
    } else if (button == 6) {
      menuSelect();
      // Check WiFi state change
      bool wifiState = menuIsWiFiEnabled();
      if (wifiState != lastWiFiState) {
        lastWiFiState = wifiState;
        if (wifiState) {
          WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
          delay(500);
          displayWiFiStatus();
        } else {
          WiFi.disconnect();
        }
      }
      // Check LED state change
      bool ledState = menuIsLedEnabled();
      if (ledState != lastLedState) {
        lastLedState = ledState;
        digitalWrite(LED_BUILTIN, ledState ? HIGH : LOW);
      }
    } else if (button == 1) {
      menuBack();
    }
  }
}

// Try to reconnect WiFi if disconnected
void reconnectWiFi() {
  // Only reconnect if WiFi is enabled in the menu
  if (menuIsWiFiEnabled() && WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
  }
}

// Display WiFi status on TFT
void displayWiFiStatus() {
  tft.setTextSize(2);
  if (menuIsWiFiEnabled()) {
    if (WiFi.status() == WL_CONNECTED) {
      displayWriteLine(&tft, 10, (String("WiFi: ") + WiFi.SSID()).c_str(), ILI9341_GREEN);
      displayWriteLine(&tft, 11, (String("IP: ") + WiFi.localIP().toString()).c_str(), ILI9341_WHITE);
    } else if (WiFi.status() == WL_DISCONNECTED) {
      displayWriteLine(&tft, 10, "WiFi: Disconnected", ILI9341_RED);
      displayWriteLine(&tft, 11, "", ILI9341_BLACK);
    } else {
      displayWriteLine(&tft, 10, "Connecting...", ILI9341_YELLOW);
      displayWriteLine(&tft, 11, "", ILI9341_BLACK);
    }
  } else {
    // WiFi disabled, clear status area
    displayWriteLine(&tft, 10, "", ILI9341_BLACK);
    displayWriteLine(&tft, 11, "", ILI9341_BLACK);
  }
}

// Read button ADC value and map to button number
int readButton() {
  int adcValue = analogRead(BUTTON_PIN);
  Serial.print("ADC Value: ");
  Serial.println(adcValue);
  if (adcValue > 3700) return 1;    // S1
  if (adcValue > 3300) return 2;    // S2
  if (adcValue > 2500) return 3;    // S3
  if (adcValue > 2200) return 4;    // S4
  if (adcValue > 1850) return 5;    // S5
  if (adcValue > 1650) return 6;    // S6
  return -1;                        // No button pressed
}
