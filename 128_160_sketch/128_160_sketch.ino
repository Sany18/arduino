// Display 1.8 TFT ST7735S 128x160 SPI:
// https://www.aliexpress.com/item/32817839166.html?spm=a2g0o.order_list.order_list_main.190.4bb01802MgViae

// Arduino Nano v3 (Old bootloader)

// Libs:
// https://downloads.arduino.cc/libraries/github.com/arduino-libraries/TFT-1.0.6.zip

// Pins:
// Arduino 	TFT Display
// 3v3 ->	LED
// 13	 -> SCK
// 11	 -> SDA
// 3	 -> AO
// 2	 -> RESET
// 4	 -> CS
// GND -> GND
// 5v	 -> VCC

// RGB565 color picker https://barth-dev.de/online/rgb565-color-picker/

#include <TFT.h>
// #include <SPI.h>
// #include <SD.h>

// pin definition
#define cs   4;
#define dc   3;
#define rst  2;

uint16_t GROUND_COLOR = 0x2823;
uint16_t LINES_COLOR = 0x4866;
uint16_t SKY_COLOR = 0x0801;
uint16_t SUN_COLOR = 0x5183;

int Y = 128;
int X = 160;

int iteration = 1;
// float a6pinValue = 0.0;
// char a6pinValueChar[20];
// char output[20];

// create an instance of the library
TFT TFTscreen = TFT(cs, dc, rst);

void setup() {
  TFTscreen.begin();
  drawSky();
  drawGround();
}

void loop() {
  drawGroundLines();
  // drawText();
  iteration++;
}

void drawText() {
  // draw current A6 pin voltage
  // a6pinValue = (float)analogRead(A6);
  // a6pinValue *= 0.0048875; // 5.0 / 1023.0
  // dtostrf(a6pinValue, 8, 2, a6pinValueChar);
  // strcpy(output, a6pinValueChar);
  // strcat(output, "V");

  // TFTscreen.fill(SKY_COLOR); // dark purple (sky color)
  // TFTscreen.rect(X - (strlen(output) * 6) - 4, 4, strlen(output) * 6, 8); // ~ 8 px is the font height
  // TFTscreen.stroke(255, 255, 255);
  // TFTscreen.text(output, X - (strlen(output) * 6) - 4, 4);
}

void drawGroundLines() {
  int amount_of_lines = 16;

  for (int i = 0; i <= amount_of_lines; i++) {
    int stepX = X / amount_of_lines;
    int stepY = Y / amount_of_lines / 2;
    float previousStepMultiplier = getStepMultiplier(iteration - 1); // 0..1
    float currentStepMultiplier = getStepMultiplier(iteration); // 0..1

    // remove previous lines
    int Y_pos_prev = pow(i + previousStepMultiplier, 2) + 64;
    TFTscreen.stroke(GROUND_COLOR); // ground color
    if (i <= amount_of_lines / 2) { // avoid lines off screen
      TFTscreen.line(0, Y_pos_prev, 160, Y_pos_prev); // horizontal lines
    }

    // draw new lines
    TFTscreen.stroke(LINES_COLOR); // lines color
    int Y_pos = pow(i + currentStepMultiplier, 2) + 64;
    if (i <= amount_of_lines / 2) { // avoid horizontal lines off screen
      TFTscreen.line(0, Y_pos, 160, Y_pos); // horizontal lines
    }

    TFTscreen.line(stepX * i * 3 - 160, 128, stepX * i, 65); // vertical lines
  }

  TFTscreen.noStroke();
}

void drawSky() {
  // sky
  TFTscreen.fill(SKY_COLOR); // dark purple (sky color)
  TFTscreen.rect(0, 0, 160, 64);

  // sun
  TFTscreen.fill(SUN_COLOR); // orange
  TFTscreen.circle(80, 50, 26);

  // sun lines
  TFTscreen.fill(SKY_COLOR); // sky color
  TFTscreen.rect(54, 22, 53, 6);
  TFTscreen.rect(54, 32, 53, 4);
  TFTscreen.rect(54, 41, 53, 3);
  TFTscreen.rect(54, 50, 53, 2);
  TFTscreen.rect(54, 58, 53, 1);

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
  TFTscreen.fill(GROUND_COLOR); // purple
  TFTscreen.rect(0, 64, 160, 64);
}

void drawMountain(int positionX, int width, int height, int color) {
  int groundHeight = 63;
  TFTscreen.fillTriangle(positionX, groundHeight, positionX + width / 2, groundHeight - height, positionX + width, groundHeight, 0x4007);
}

float getStepMultiplier(int _step) { // 0...9999 to 0...1
  int a = _step % 10;
  return a * 0.1;
}
