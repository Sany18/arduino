#include "clock.h"

// Time tracking variables
static unsigned long timeEpoch = 0;  // Current time in seconds since Unix epoch
static unsigned long timeMillisOffset = 0;  // millis() value when time was last set
static TM1637* display = nullptr;

// Initialize clock with display pointer
void initClock(TM1637* displayPtr) {
  display = displayPtr;
}

// Get current time in seconds since Unix epoch
unsigned long getCurrentTime() {
  if (timeEpoch == 0) {
    return 0;  // Time not set yet
  }
  unsigned long elapsed = (millis() - timeMillisOffset) / 1000;
  return timeEpoch + elapsed;
}

// Set current time in seconds since Unix epoch
void setCurrentTime(unsigned long epoch) {
  timeEpoch = epoch;
  timeMillisOffset = millis();
}

// Display time on TM1637 display in HH.MM.SS format
void displayTime() {
  if (display == nullptr) return;
  
  unsigned long currentTime = getCurrentTime();
  
  if (currentTime == 0) {
    // Time not set yet, display 0
    display->displayInt(0);
    return;
  }
  
  // Calculate hours, minutes, seconds
  unsigned long totalSeconds = currentTime % 86400;  // Seconds in current day
  int hours = totalSeconds / 3600;
  int minutes = (totalSeconds % 3600) / 60;
  int seconds = totalSeconds % 60;
  
  // Create 6-digit array similar to the displayTime example
  uint8_t data[6];
  data[5] = hours / 10;
  data[4] = hours % 10;
  data[3] = minutes / 10;
  data[2] = minutes % 10;
  data[1] = seconds / 10;
  data[0] = seconds % 10;
  
  // Add dots using the 0x80 flag (position 4 for hours, position 2 for minutes)
  data[4] |= 0x80;  // Dot after hours
  data[2] |= 0x80;  // Dot after minutes
  
  // Display using displayRaw
  display->displayRaw(data, -1);
}
