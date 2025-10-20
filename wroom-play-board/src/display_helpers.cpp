#include <Adafruit_ILI9341.h>
#include "display_helpers.h"

// 12 lines
void displayWriteLine(Adafruit_ILI9341* tft, int line, const char* text, uint16_t color) {
  if (!tft) return;
  int lineHeight = 20;
  int y = 5 + line * lineHeight;
  tft->fillRect(0, y, tft->width(), lineHeight, 0x0000); // ILI9341_BLACK
  tft->setCursor(0, y);
  tft->setTextColor(color);
  tft->setTextSize(2);
  tft->println(text);
}
