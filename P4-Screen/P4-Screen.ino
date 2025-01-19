#include "oled_display.h"

String incomingMessage = ""; // Variable para almacenar el mensaje entrante
String hour1 = "No data";
String coord1 = "No data";
String hour2 = "No data";
String coord2 = "No data";

void setup() {
  Serial.begin(9600);
  turnOnOLED();
  updateScreen();
}

void loop() {
  // Verificar si hay datos disponibles en el puerto Serial
  if (Serial.available()) {
    char incomingChar = Serial.read(); // Leer un carácter
    if (incomingChar == '\n') {        // Si es un fin de línea
      processMessage(incomingMessage); // Procesar el mensaje recibido
      incomingMessage = "";            // Limpiar la variable
    } else {
      incomingMessage += incomingChar; // Acumular los caracteres en la variable
    }
  }
}

void processMessage(String message) {
  // Procesar los mensajes recibidos según el tópico
  if (message.startsWith("trabajo_curso/coord~1-")) {
    coord1 = message.substring(22);
    updateScreen();
  } else if (message.startsWith("trabajo_curso/ultrasonic~1-")) {
    hour1 = message.substring(27);
    updateScreen();
  } else if (message.startsWith("trabajo_curso/coord~2-")) {
    coord2 = message.substring(22);
    updateScreen();
  } else if (message.startsWith("trabajo_curso/ultrasonic~2-")) {
    hour2 = message.substring(27);
    updateScreen();
  }else {
    return;
  }
}

void updateScreen(){
  drawOLED(hour1,coord1,hour2,coord2);
}
