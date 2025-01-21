/*
 * Firmware de dispositivo publisher - Trabajo de Curso IC 2024/25
 * 
 * Funciones:
 * - Lectura de sensores Devantech SRF02 y Arduino MKR GPS Shield
 * - Publicación de datos por Serial.
 * 
 * Librerías utilizadas:
 * - Wire             : Arduino's I2C library
 * - Arduino_MKRGPS   : Read location from MKR GPS Shield
 * - Arduino_BQ24195  : Arduino's Power Management IC library
 * - LoRa             : Modified LoRa library (data send & receive)
 *  
 */




//libraries
#include <Wire.h>
#include <Arduino_MKRGPS.h>
#include <Arduino_PMIC.h>
#include <LoRa.h>
#include <SPI.h>


//variables & constants
#define TX_LAPSE_MS          5000
#define THRESHOLD            20

byte open_f = 0;

typedef struct {
  float latitude;
  float longitude;
} locationStruct;

locationStruct myLocation;

//LoRa constant variables
const uint8_t localAddress = 0x54;     // Dirección de este dispositivo
uint8_t destination = 0x52;            // Dirección de destino, 0xFF es la dirección de broadcast

volatile bool txDoneFlag = true;       // Flag para indicar cuando ha finalizado una transmisión
volatile bool transmitting = false;

typedef struct {
  uint8_t bandwidth_index;
  uint8_t spreadingFactor;
  uint8_t codingRate;
  uint8_t txPower; 
} LoRaConfig_t;

double bandwidth_kHz[10] = {7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3,
                            41.7E3, 62.5E3, 125E3, 250E3, 500E3 };

LoRaConfig_t thisNodeConf = { 10, 7, 5, 2};

uint16_t msgCount = 0;
uint16_t last_message = 0;



//SRF02's constant variables
#define SRF02_I2C_ADDRESS byte((0xE2)>>1) 
#define SRF02_I2C_INIT_DELAY 100 // in milliseconds
#define SRF02_RANGING_DELAY 70 // milliseconds

// LCD05's command related definitions
#define COMMAND_REGISTER byte(0x00)
#define SOFTWARE_REVISION byte(0x00)
#define RANGE_HIGH_BYTE byte(2)
#define RANGE_LOW_BYTE byte(3)
#define AUTOTUNE_MINIMUM_HIGH_BYTE byte(4)
#define AUTOTUNE_MINIMUM_LOW_BYTE byte(5)

// SRF02's command codes
#define REAL_RANGING_MODE_INCHES    byte(80)
#define REAL_RANGING_MODE_CMS       byte(81)
#define REAL_RANGING_MODE_USECS     byte(82)
#define FAKE_RANGING_MODE_INCHES    byte(86)
#define FAKE_RANGING_MODE_CMS       byte(87)
#define FAKE_RANGING_MODE_USECS     byte(88)
#define TRANSMIT_8CYCLE_40KHZ_BURST byte(92)
#define FORCE_AUTOTUNE_RESTART      byte(96)
#define ADDRESS_CHANGE_1ST_SEQUENCE byte(160)
#define ADDRESS_CHANGE_3RD_SEQUENCE byte(165)
#define ADDRESS_CHANGE_2ND_SEQUENCE byte(170)

inline void write_command(byte address,byte command)
{ 
  Wire.beginTransmission(address);
  Wire.write(COMMAND_REGISTER); 
  Wire.write(command); 
  Wire.endTransmission();
}

byte read_register(byte address,byte the_register)
{
  Wire.beginTransmission(address);
  Wire.write(the_register);
  Wire.endTransmission();
  
  // getting sure the SRF02 is not busy
  Wire.requestFrom(address,byte(1));
  while(!Wire.available()) { /* idle */ }
  return Wire.read();
} 

void SRF02_data_read() {
  write_command(SRF02_I2C_ADDRESS,REAL_RANGING_MODE_CMS);
  delay(SRF02_RANGING_DELAY);
  
  byte high_byte_range=read_register(SRF02_I2C_ADDRESS,RANGE_HIGH_BYTE);
  byte low_byte_range=read_register(SRF02_I2C_ADDRESS,RANGE_LOW_BYTE);
  byte high_min=read_register(SRF02_I2C_ADDRESS,AUTOTUNE_MINIMUM_HIGH_BYTE);
  byte low_min=read_register(SRF02_I2C_ADDRESS,AUTOTUNE_MINIMUM_LOW_BYTE);
  

  //Alarma (operador ternario)
  open_f = (int((high_byte_range<<8) | low_byte_range) <= THRESHOLD) ? 1 : 0;
  Serial.print("Puerta abierta?" ); Serial.println(open_f);

  /* Método simple
  if(int((high_byte_range<<8) | low_byte_range) <= 30){
    open_f = 1;
  } else{
    open_f = 0;
  }
  */
 
}


void GPS_data_read() {
  GPS.wakeup();
  // Check if there is new GPS data available
  if (GPS.available()) {
    // Read GPS data
    float latitude = GPS.latitude();
    float longitude = GPS.longitude();
    Serial.print("Latitude: "); Serial.println(latitude, 6);
    Serial.print("Longitude: "); Serial.println(longitude, 6);
  
    // Store GPS data
    myLocation.latitude = latitude;
    myLocation.longitude = longitude;
  }
  GPS.standby();
}


void setUpLoRa(LoRaConfig_t conf){
  LoRa.idle();
  LoRa.setSignalBandwidth(long(bandwidth_kHz[conf.bandwidth_index])); 
                                  // 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3
                                  // 41.7E3, 62.5E3, 125E3, 250E3, 500E3 
                                  // Multiplicar por dos el ancho de banda
                                  // supone dividir a la mitad el tiempo de Tx
                                  
  LoRa.setSpreadingFactor(conf.spreadingFactor);     
                                  // [6, 12] Aumentar el spreading factor incrementa 
                                  // de forma significativa el tiempo de Tx
                                  // SPF = 6 es un valor especial
                                  // Ver tabla 12 del manual del SEMTECH SX1276
  LoRa.setTxPower(conf.txPower, PA_OUTPUT_PA_BOOST_PIN); 
                                  // Rango [2, 20] en dBm
                                  // Importante seleccionar un valor bajo para pruebas
                                  // a corta distancia y evitar saturar al receptor
  // Nótese que la recepción está activada a partir de este punto
  LoRa.receive();
}

void sendMessage(uint8_t* payload, uint8_t payloadLength, uint16_t msgCount) 
{
  while(!LoRa.beginPacket()) {            // Comenzamos el empaquetado del mensaje
    delay(10);                            // 
  }
  LoRa.write(destination);                // Añadimos el ID del destinatario
  LoRa.write(localAddress);               // Añadimos el ID del remitente
  LoRa.write((uint8_t)(msgCount >> 7));   // Añadimos el Id del mensaje (MSB primero)
  LoRa.write((uint8_t)(msgCount & 0xFF)); 
  LoRa.write(payloadLength);              // Añadimos la longitud en bytes del mensaje
  LoRa.write(payload, (size_t)payloadLength); // Añadimos el mensaje/payload 
  LoRa.endPacket(true);                   // Finalizamos el paquete, pero no esperamos a
                                          // finalice su transmisión

  
}

void TxFinished()
{
  txDoneFlag = true;
}

void printBinaryPayload(uint8_t * payload, uint8_t payloadLength)
{
  for (int i = 0; i < payloadLength; i++) {
    Serial.print((payload[i] & 0xF0) >> 4, HEX);
    Serial.print(payload[i] & 0x0F, HEX);
    Serial.print(" ");
  }
  Serial.println("");
}

void setup() {
  //serial begin
  Serial.begin(9600);
  while (!Serial); 

  //setup SRF02
  Serial.println("initializing Wire interface ...");
  Wire.begin();
  delay(SRF02_I2C_INIT_DELAY);  
  
  byte software_revision=read_register(SRF02_I2C_ADDRESS,SOFTWARE_REVISION);
  Serial.print("SFR02 ultrasonic range finder in address 0x");
  Serial.print(SRF02_I2C_ADDRESS,HEX); Serial.print("(0x");
  Serial.print(software_revision,HEX); Serial.println(")");

  //setup MKR GPS Shield (default I2C mode)
  if (!GPS.begin()) {
    Serial.println("Failed to initialize GPS!");
    while (1);
  } else {
    Serial.println("Initialization of GPS succeded!");
  }
  
  if (!init_PMIC()) {
    Serial.println("Initilization of BQ24195L failed!");
  }
  else {
    Serial.println("Initilization of BQ24195L succeeded!");
  }

  //setup LoRa
  if (!LoRa.begin(868E6)) {      // Initicializa LoRa a 868 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                
  }
  last_message = millis();
  // Configuramos algunos parámetros de la radio
  LoRa.setCodingRate4(thisNodeConf.codingRate);         
                                  // [5, 8] 5 da un tiempo de Tx menor
  LoRa.setSyncWord(0x12);         // Palabra de sincronización privada por defecto para SX127X 
                                  // Usaremos la palabra de sincronización para crear diferentes
                                  // redes privadas por equipos
  LoRa.setPreambleLength(8);      // Número de símbolos a usar como preámbulo
  
  // Activamos el callback que nos indicará cuando ha finalizado la 
  // transmisión de un mensaje
  LoRa.onTxDone(TxFinished);
  setUpLoRa(thisNodeConf);
  Serial.println("LoRa init succeeded.\n");
}

void loop() {
  if (millis() - last_message > TX_LAPSE_MS) {
    last_message = millis();
    // Lectura de los sensores
    SRF02_data_read();
    GPS_data_read();
    // Construcción del payload del mensaje
    uint8_t payload[50];
    uint8_t payloadLength = 1;
    payload[0] = open_f;
    // Añadir myLocation.latitude al payload
    memcpy(payload + payloadLength, &myLocation.latitude, sizeof(myLocation.latitude));
    payloadLength += sizeof(myLocation.latitude);
    // Añadir myLocation.longitude al payload
    memcpy(payload + payloadLength, &myLocation.longitude, sizeof(myLocation.longitude));
    payloadLength += sizeof(myLocation.longitude);
    // Envío del mensaje
    txDoneFlag = false;  
    sendMessage(payload, payloadLength, msgCount);
    printBinaryPayload(payload, payloadLength);             
    msgCount++;
    while (!txDoneFlag) {
      continue;
    }
  }
}




