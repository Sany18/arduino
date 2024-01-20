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
// SCL (SCLK) = GPIO9
// SDA (MOSI) = GPIO8
// RES = RST
// DC  = GPIO3
// CS  = GPIO2
// BLK = +3.3V

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

#define TFT_CS         2
#define TFT_RST        4
#define TFT_DC         3
#define TFT_MOSI_SDA   8
#define TFT_SCLK_SCL   9

Adafruit_ST7789 TFTscreen = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI_SDA, TFT_SCLK_SCL, TFT_RST);

uint16_t GROUND_COLOR = 0x2823;
uint16_t LINES_COLOR = 0x4866;
uint16_t SKY_COLOR = 0x0801;
uint16_t SUN_COLOR = 0x5183;

int maxY = 170;
int maxX = 320;

int iteration = 1;

void setup() {
  TFTscreen.init(maxY, maxX);
  drawSky();
  drawGround();
}

void loop() {
  drawGroundLines();
  iteration++;
}

void drawGroundLines() {
  int amount_of_lines = 16;

  for (int i = 0; i <= amount_of_lines; i++) {
    int stepX = maxX / amount_of_lines;
    int stepY = maxY / amount_of_lines / 2;
    float previousStepMultiplier = getStepMultiplier(iteration - 1); // 0..1
    float currentStepMultiplier = getStepMultiplier(iteration); // 0..1

    // remove previous lines
    int Y_pos_prev = pow(i + previousStepMultiplier, 2) + maxY/2;
    if (i <= amount_of_lines / 2) { // avoid lines off screen
      TFTscreen.drawLine(0, Y_pos_prev, maxX, Y_pos_prev, GROUND_COLOR); // horizontal lines
    }

    // draw new lines
    int Y_pos = pow(i + currentStepMultiplier, 2) + maxY/2;
    if (i <= amount_of_lines / 2) { // avoid horizontal lines off screen
      TFTscreen.drawLine(0, Y_pos, maxX, Y_pos, LINES_COLOR); // horizontal lines
    }

    TFTscreen.drawLine(stepX * i * 3 - maxX, maxY, stepX * i, (maxY/2 + 1), LINES_COLOR); // vertical lines
  }
}

void drawSky() {
  // sky
  TFTscreen.fillRect(0, 0, maxX, maxY/2, SKY_COLOR);

  // sun
  TFTscreen.drawCircle(80, 50, 26, SUN_COLOR);

  // sun lines
  TFTscreen.fillRect(54, 22, 53, 6, SKY_COLOR);
  TFTscreen.fillRect(54, 32, 53, 4, SKY_COLOR);
  TFTscreen.fillRect(54, 41, 53, 3, SKY_COLOR);
  TFTscreen.fillRect(54, 50, 53, 2, SKY_COLOR);
  TFTscreen.fillRect(54, 58, 53, 1, SKY_COLOR);

  // left mountains
  int groundHeight = 63;
  drawMountain(-10, 50, 12, 0x4007);
  drawMountain(15, 30, 18, 0x4007);
  drawMountain(35, 20, 10, 0x3005);

  // right mountains
  drawMountain(102, 20, 10, 0x4007);
  drawMountain(130, 30, 20, 0x0801);
  drawMountain(115, 18, 5, 0x2001);
}

void drawGround() {
  TFTscreen.fillRect(0, maxY/2, maxX, maxY/2, GROUND_COLOR);
}

void drawMountain(int positionX, int width, int height, int color) {
  int groundHeight = maxY/2 - 1;
  TFTscreen.drawTriangle(positionX, groundHeight, positionX + width / 2, groundHeight - height, positionX + width, groundHeight, 0x4007);
}

float getStepMultiplier(int _step) { // 0...9999 to 0...1
  int a = _step % 10;
  return a * 0.1;
}
