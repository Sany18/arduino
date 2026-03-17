#ifndef CLOCK_H
#define CLOCK_H

#include <Arduino.h>
#include <TM1637.h>

// Initialize clock (pass the display object)
void initClock(TM1637* displayPtr);

// Get current time in seconds since Unix epoch
unsigned long getCurrentTime();

// Set current time in seconds since Unix epoch
void setCurrentTime(unsigned long epoch);

// Display time on TM1637 display in HH.MM.SS format
void displayTime();

#endif
