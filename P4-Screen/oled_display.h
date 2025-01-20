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
void setNewOLEDValues(String lat1grados = "", String lat1minutos = "", 
                      String lon1grados = "", String lon1minutos = "", 
                      String lat2grados = "", String lat2minutos = "", 
                      String lon2grados = "", String lon2minutos = "", 
                      String newSensor1Time = "", String newSensor2Time = "");
void turnOnOLED();
void turnOffOLED();

#endif // OLED_DISPLAY_H