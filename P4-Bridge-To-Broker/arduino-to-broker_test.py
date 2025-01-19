import serial
import paho.mqtt.client as mqtt

# Configuración del puerto Serial
ser = serial.Serial('COM3', 9600)  # Reemplaza 'COMx' con el puerto correcto de tu Arduino

# Configuración del cliente MQTT
mqtt_broker = "localhost"  # Dirección de tu broker Mosquitto
mqtt_port = 1883
mqtt_topic = "arduino/test"

client = mqtt.Client()

# Conectar al broker
client.connect(mqtt_broker, mqtt_port, 60)

while True:
    # Leer mensaje del puerto Serial
    if ser.in_waiting > 0:
        message = ser.readline().decode('utf-8').strip()
        
        # Publicar el mensaje en el broker MQTT
        client.publish(mqtt_topic, message)
        print(f"Mensaje enviado al broker MQTT: {message}")

    # Mantener la conexión MQTT
    client.loop()
