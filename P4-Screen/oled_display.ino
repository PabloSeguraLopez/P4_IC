#include "oled_display.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool oled_flag = true;

/**
 * @brief Actualiza la pantalla OLED con información de los sensores.
 */
void drawOLED(String hour1, String coord1, String hour2, String coord2){
  if (oled_flag){
    display.clearDisplay();
    drawSensorInformation(hour1,coord1,hour2,coord2);
  }
}

/**
 * @brief Dibuja información de los GNSS y ultrasonidos en la pantalla OLED.
 */
void drawSensorInformation(String hour1, String coord1, String hour2, String coord2) {
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  // Arduino with ID = 1
  display.print(F("1:"));
  display.setTextColor(SSD1306_WHITE);
  display.print("OPEN:");
  display.println(hour1);
  display.print("POS:");
  display.println(coord1);
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);        // Draw white text
  // Arduino with ID = 2
  display.print(F("2:"));
  display.setTextColor(SSD1306_WHITE);
  display.print("OPEN:");
  display.println(hour2);
  display.print("POS:");
  display.println(coord2);

  display.display();
}

void turnOffOLED(){
  display.clearDisplay();
  display.display();
  oled_flag = false;
}

void turnOnOLED(){
  oled_flag = true;
  display.clearDisplay();
}