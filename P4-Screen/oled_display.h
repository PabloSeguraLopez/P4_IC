#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RTCZero.h>

// Display constants
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3D

// Global OLED display variable
extern Adafruit_SSD1306 display;

// Function declarations
void initDisplay();
void drawOLED();
void drawSensorsInformation();
void setNewOLEDValues(int lat1grados = -1, int lat1minutos = -1, 
                      int lon1grados = -1, int lon1minutos = -1, 
                      int lat2grados = -1, int lat2minutos = -1, 
                      int lon2grados = -1, int lon2minutos = -1, 
                      String newSensor1Time = "", String newSensor2Time = "");
void turnOnOLED();
void turnOffOLED();

#endif // OLED_DISPLAY_H