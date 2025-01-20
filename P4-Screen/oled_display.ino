#include "oled_display.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool oled_flag = true;

String lat1_grados, lat1_minutos, lon1_grados, lon1_minutos;
String lat2_grados, lat2_minutos, lon2_grados, lon2_minutos;
String sensor1Time, sensor2Time;

void initDisplay(){
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // No continuar si hay fallo
  }
}

/**
 * @brief Actualiza la pantalla OLED con información de los sensores.
 */
void drawOLED(){
  if (oled_flag){
    display.clearDisplay();

    drawSensorsInformation();
  }
}

/**
 * @brief Dibuja información del sensor 1 en la pantalla OLED.
 */
void drawSensorsInformation() {
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setCursor(0,0);
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);        // Draw white text
  display.println(F("Sensor 1"));
  display.setTextColor(SSD1306_WHITE);    
  display.print(F("Open: "));
  display.println(sensor1Time);
  display.print(F("  Lat: ")); display.print(lat1_grados); display.print((char)248); display.print(F(" ")); display.print(lat1_minutos); display.println(F("'"));
  display.print(F("  Lon: ")); display.print(lon1_grados); display.print((char)248); display.print(F(" ")); display.print(lon1_minutos); display.println(F("'"));
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);        // Draw white text
  display.println(F("Sensor 2"));
  display.setTextColor(SSD1306_WHITE);    
  display.print(F("Open: "));
  display.println(sensor2Time);
  display.print(F("  Lat: ")); display.print(lat2_grados); display.print((char)248); display.print(F(" ")); display.print(lat2_minutos); display.println(F("'"));
  display.print(F("  Lon: ")); display.print(lon2_grados); display.print((char)248); display.print(F(" ")); display.print(lon2_minutos); display.println(F("'"));
  display.display();
}

void setNewOLEDValues(String lat1grados, String lat1minutos, 
                      String lon1grados, String lon1minutos, 
                      String lat2grados, String lat2minutos, 
                      String lon2grados, String lon2minutos, 
                      String newSensor1Time, String newSensor2Time) {
    
  // Solo actualizar si el valor no es ""
  if (lat1grados != "") lat1_grados = lat1grados;
  if (lat1minutos != "") lat1_minutos = lat1minutos;
  if (lon1grados != "") lon1_grados = lon1grados;
  if (lon1minutos != "") lon1_minutos = lon1minutos;
  if (lat2grados != "") lat2_grados = lat2grados;
  if (lat2minutos != "") lat2_minutos = lat2minutos;
  if (lon2grados != "") lon2_grados = lon2grados;
  if (lon2minutos != "") lon2_minutos = lon2minutos;

  if (newSensor1Time != "") sensor1Time = newSensor1Time;
  if (newSensor2Time != "") sensor2Time = newSensor2Time;


  drawOLED();
}

void turnOffOLED(){
  display.clearDisplay();
  display.display();
  oled_flag = false;
}

void turnOnOLED(){
  oled_flag = true;
  drawOLED();
}
