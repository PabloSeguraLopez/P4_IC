#include <SPI.h>             
#include <LoRa.h>
#include <Arduino_PMIC.h>
void setup() {
  Serial.begin(9600);
}

void loop() {
  // Simulación de datos de temperatura y sensor ultrasónico
  float temperatura = random(20, 30) + random(0, 99) / 100.0; // Genera temperatura simulada
  int distancia = random(5, 50); // Genera distancia simulada en cm

  // Publicar en el tópico "trabajo_curso/temp"
  Serial.print("trabajo_curso/temp:");
  Serial.println(temperatura);

  // Publicar en el tópico "trabajo_curso/ultrasonic"
  Serial.print("trabajo_curso/ultrasonic:");
  Serial.println(distancia);

  delay(2000); // Esperar 2 segundos antes de enviar el próximo conjunto de datos
}
