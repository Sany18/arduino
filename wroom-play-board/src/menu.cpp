#include <stdint.h>
#include "MyILI9341.h"
#include "menu.h"

#define MENU_SIZE 2

static MenuItem menuItems[MENU_SIZE];
static int selected = 0;
static MyILI9341* menuTft = nullptr;

#include "display_helpers.h"
static bool wifiEnabled = false;
static bool ledEnabled = false;

void menuActionToggleWiFi() {
  wifiEnabled = !wifiEnabled;
  displayWriteLine(menuTft, 4, wifiEnabled ? "WiFi Enabled" : "WiFi Disabled", ILI9341_YELLOW);
  menuRender();
}

void menuActionToggleLED() {
  ledEnabled = !ledEnabled;
  displayWriteLine(menuTft, 4, ledEnabled ? "LED Enabled" : "LED Disabled", ILI9341_YELLOW);
  menuRender();
}
// Getter functions for Main.cpp to check menu state
bool menuIsWiFiEnabled() { return wifiEnabled; }
bool menuIsLedEnabled() { return ledEnabled; }

void menuInit(MyILI9341* tft) {
  menuTft = tft;
  ledEnabled = false;
  menuItems[0] = {"WiFi: On/Off", menuActionToggleWiFi};
  menuItems[1] = {"Back LED: On/Off", menuActionToggleLED};
  selected = 0;
  menuRender();
}

void menuUp() {
  if (selected > 0) selected--;
  menuRender();
}

void menuDown() {
  if (selected < MENU_SIZE - 1) selected++;
  menuRender();
}

void menuSelect() {
  if (selected >= 0 && selected < MENU_SIZE) {
    menuItems[selected].action();
  }
}

void menuBack() {
  // Optionally implement back logic (e.g., exit menu)
  displayWriteLine(menuTft, 4, "Back pressed", ILI9341_WHITE);
}

void menuRender() {
  if (!menuTft) return;
  int menuHeight = MENU_SIZE * 20 + 10;
  // Only clear the menu area, not the whole screen
  for (int i = 0; i < MENU_SIZE; ++i) {
    char lineText[40];
    if (i == selected) {
      snprintf(lineText, sizeof(lineText), "> %s %s", menuItems[i].label,
        (i == 0) ? (wifiEnabled ? "[ON]" : "[OFF]") : (i == 1) ? (ledEnabled ? "[ON]" : "[OFF]") : "");
  displayWriteLine(menuTft, i, lineText, ILI9341_GREEN);
    } else {
      snprintf(lineText, sizeof(lineText), "  %s %s", menuItems[i].label,
        (i == 0) ? (wifiEnabled ? "[ON]" : "[OFF]") : (i == 1) ? (ledEnabled ? "[ON]" : "[OFF]") : "");
  displayWriteLine(menuTft, i, lineText, ILI9341_WHITE);
    }
  }
}

int menuGetSelected() {
  return selected;
}
