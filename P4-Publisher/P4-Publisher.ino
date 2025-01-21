#include <SPI.h>             
#include <LoRa.h>
#include <Arduino_PMIC.h>
#include <time.h>
#include <ArduinoLowPower.h>


#define TX_LAPSE_MS          5000
#define MESSAGES_LOSS_LIMIT 5

const uint8_t localAddress = 0x52;     // Dirección de este dispositivo
uint8_t destination = 0x53;            // Dirección de destino, 0xFF es la dirección de broadcast

volatile bool txDoneFlag = true;       // Flag para indicar cuando ha finalizado una transmisión

// Estructura para almacenar la configuración de la radio
typedef struct {
  uint8_t bandwidth_index;
  uint8_t spreadingFactor;
  uint8_t codingRate;
  uint8_t txPower; 
} LoRaConfig_t;

double bandwidth_kHz[10] = {7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3,
                            41.7E3, 62.5E3, 125E3, 250E3, 500E3 };

LoRaConfig_t thisNodeConf   = { 10, 7, 5, 2};

// rtc object to manage the real time clock
RTCZero rtc;

// --------------------------------------------------------------------
// Setup function
// --------------------------------------------------------------------
void setup() 
{
  Serial.begin(115200);
  // Inicializamos el RTC
  rtc.begin();
  setDateTime(__DATE__, __TIME__);

  if (!init_PMIC()) {
    Serial.println("Initilization of BQ24195L failed!");
  }
  else {
    Serial.println("Initilization of BQ24195L succeeded!");
  }

  if (!LoRa.begin(868E6)) {      // Initicializa LoRa a 868 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                
  }

  // Configuramos algunos parámetros de la radio
  LoRa.setSignalBandwidth(long(bandwidth_kHz[thisNodeConf.bandwidth_index])); 
                                  // 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3
                                  // 41.7E3, 62.5E3, 125E3, 250E3, 500E3 
                                  // Multiplicar por dos el ancho de banda
                                  // supone dividir a la mitad el tiempo de Tx
                                  
  LoRa.setSpreadingFactor(thisNodeConf.spreadingFactor);     
                                  // [6, 12] Aumentar el spreading factor incrementa 
                                  // de forma significativa el tiempo de Tx
                                  // SPF = 6 es un valor especial
                                  // Ver tabla 12 del manual del SEMTECH SX1276
  
  LoRa.setCodingRate4(thisNodeConf.codingRate);         
                                  // [5, 8] 5 da un tiempo de Tx menor
                                  
  LoRa.setTxPower(thisNodeConf.txPower, PA_OUTPUT_PA_BOOST_PIN); 
                                  // Rango [2, 20] en dBm
                                  // Importante seleccionar un valor bajo para pruebas
                                  // a corta distancia y evitar saturar al receptor
  LoRa.setSyncWord(0x12);         // Palabra de sincronización privada por defecto para SX127X 
                                  // Usaremos la palabra de sincronización para crear diferentes
                                  // redes privadas por equipos
  LoRa.setPreambleLength(8);      // Número de símbolos a usar como preámbulo

  
  // Indicamos el callback para cuando se reciba un paquete
  LoRa.onReceive(onReceive);
  
  // Activamos el callback que nos indicará cuando ha finalizado la 
  // transmisión de un mensaje
  LoRa.onTxDone(TxFinished);

  // Nótese que la recepción está activada a partir de este punto
  LoRa.receive();

  Serial.println("LoRa init succeeded.\n");
}

// --------------------------------------------------------------------
// Loop function
// --------------------------------------------------------------------
void loop() 
{
}


// --------------------------------------------------------------------
// Receiving message function
// --------------------------------------------------------------------
void onReceive(int packetSize) 
{
  if (packetSize == 0) return;          // Si no hay mensajes, retornamos

  // Leemos los primeros bytes del mensaje
  uint8_t buffer[10];                   // Buffer para almacenar el mensaje
  int recipient = LoRa.read();          // Dirección del destinatario
  uint8_t sender = LoRa.read();         // Dirección del remitente
                                        // msg ID (High Byte first)
  uint16_t incomingMsgId = ((uint16_t)LoRa.read() << 7) | 
                            (uint16_t)LoRa.read();
  
  uint8_t incomingLength = LoRa.read(); // Longitud en bytes del mensaje
  
  uint8_t receivedBytes = 0;            // Leemos el mensaje byte a byte
  while (LoRa.available() && (receivedBytes < uint8_t(sizeof(buffer)-1))) {            
    buffer[receivedBytes++] = (char)LoRa.read();
  }
  
  if (incomingLength != receivedBytes) {// Verificamos la longitud del mensaje
    return;                             
  }

  // Verificamos si se trata de un mensaje en broadcast o es un mensaje
  // dirigido específicamente a este dispositivo.
  // Nótese que este mecanismo es complementario al uso de la misma
  // SyncWord y solo tiene sentido si hay más de dos receptores activos
  // compartiendo la misma palabra de sincronización
  if ((recipient & localAddress) != localAddress ) {
    return;
  }

  // Imprimimos los detalles del mensaje recibido
  Serial.println("Received from: 0x" + String(sender, HEX));
  Serial.println("Sent to: 0x" + String(recipient, HEX));

  // Actualizamos remoteNodeConf y lo mostramos
  if (receivedBytes == 9) {
    String id = sender == 0x53 ? "1" : "2";
    byte open_f = buffer[0];
    float latitude = 0;
    memcpy(&latitude, buffer + 1, sizeof(latitude));
    float longitude = 0;
    memcpy(&longitude, buffer + 5, sizeof(longitude));
    publish(latitude, longitude, id, open_f);
  }
}

void TxFinished()
{
  txDoneFlag = true;
}

void publish(float latitude, float longitude, String id, byte open_f)
{
  if (open_f > 0){
    Serial.print("trabajo_curso/ultrasonic~");Serial.print(id);Serial.print("-");Serial.println(getTime());
  }
  int lat_degrees = (int)latitude;
  int lat_minutes = takeMinutes(latitude);
  int lon_degrees = (int)longitude;
  int lon_minutes = takeMinutes(longitude);
  Serial.print("trabajo_curso/coord~");Serial.print(id);Serial.print("-");
  // Grados de latitud
  Serial.print(lat_degrees < 10 ? String("0"+String(lat_degrees)) : String(lat_degrees));
  //Separador
  Serial.print("o");
  // Minutos de latitud
  Serial.print(lat_minutes < 10 ? String("0"+String(lat_minutes)) : String(lat_minutes));
  //Separador
  Serial.print(",");
  //Grados de longitud
  Serial.print(lon_degrees < 10 ? String("0"+String(lon_degrees)) : String(lon_degrees));
  //Separador
  Serial.print("o");
  // Minutos de longitud
  Serial.println(lon_minutes < 10 ? String("0"+String(lon_minutes)) : String(lon_minutes));
}


int takeMinutes(float decimalDegrees) {
  // Calcular los minutos (resto de los grados multiplicado por 60)
  float fractionalDegrees = decimalDegrees - (float)(int)decimalDegrees;
  return (int)(fractionalDegrees * 60);
}

bool setDateTime(const char * date_str, const char * time_str)
{
  char month_str[4];
  char months[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug",
                        "Sep", "Oct", "Nov", "Dec"};
  uint16_t i, mday, month, hour, min, sec, year;

  if (sscanf(date_str, "%3s %hu %hu", month_str, &mday, &year) != 3) return false;
  if (sscanf(time_str, "%hu:%hu:%hu", &hour, &min, &sec) != 3) return false;

  for (i = 0; i < 12; i++) {
    if (!strncmp(month_str, months[i], 3)) {
      month = i + 1;
      break;
    }
  }
  if (i == 12) return false;
  
  rtc.setTime((uint8_t)hour, (uint8_t)min, (uint8_t)sec);
  rtc.setDate((uint8_t)mday, (uint8_t)month, (uint8_t)(year - 2000));
  return true;
}

String getTime()
{
  // Obtenemos el tiempo Epoch, segundos desde el 1 de enero de 1970
  time_t epoch = rtc.getEpoch();

  // Convertimos a la forma habitual de fecha y hora
  struct tm stm;
  gmtime_r(&epoch, &stm);
  
  // Devolvemos la hora en formato HH:MM:SS
  char time[8];
  sprintf(time, "%02d:%02d:%02d", stm.tm_hour, stm.tm_min, stm.tm_sec);
  return String(time);
}

