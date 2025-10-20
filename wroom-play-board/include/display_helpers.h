#pragma once
#include <Adafruit_ILI9341.h>
#include <stdint.h>

void displayWriteLine(Adafruit_ILI9341* tft, int line, const char* text, uint16_t color = 0xFFFF);
