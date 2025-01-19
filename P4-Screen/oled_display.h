#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Display constants
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3D

// Global OLED display variable
extern Adafruit_SSD1306 display;

// Function declarations
void drawOLED(String hour1, String coord1, String hour2, String coord2);
void drawSensorInformation(String hour1, String coord1, String hour2, String coord2);
void turnOnOLED();
void turnOffOLED();

#endif // OLED_DISPLAY_H