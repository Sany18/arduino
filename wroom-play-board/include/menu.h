#pragma once
#include "MyILI9341.h"
#include <Adafruit_ILI9341.h>

// Menu item structure
struct MenuItem {
  const char* label;
  void (*action)();
};

void menuInit(MyILI9341* tft);
void menuUp();
void menuDown();
void menuSelect();
void menuRender();
int menuGetSelected();
void menuBack();
bool menuIsWiFiEnabled();
bool menuIsLedEnabled();
