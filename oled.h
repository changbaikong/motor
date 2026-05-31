#ifndef OLED_H
#define OLED_H

#include "Arduino.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

extern Adafruit_SSD1306 OLED;

void initOLED();
// void updateDisplay();
// void showAngleDisplay();
void showStartupScreen();
void showStatusDisplay();

#endif
