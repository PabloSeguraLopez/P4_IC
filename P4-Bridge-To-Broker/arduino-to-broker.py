import serial
import paho.mqtt.client as mqtt

# Configuración de los puertos Serial
arduino_publisher = serial.Serial('COM3', 9600)
arduino_subscriber = serial.Serial('COM8', 9600)

# Configuración del cliente MQTT
mqtt_broker = "localhost"  # Dirección del broker MQTT
mqtt_port = 1883

# Tópicos
topic_coord = "trabajo_curso/coord"
topic_ultrasonic = "trabajo_curso/ultrasonic"

# Crear cliente MQTT
client = mqtt.Client()

# Función de callback cuando se recibe un mensaje de un tópico
def on_message(client, userdata, msg):
    # Enviar el mensaje recibido por MQTT al suscriptor Arduino
    message = f"{msg.topic}~{msg.payload.decode('utf-8')}\n"
    arduino_subscriber.write(message.encode('utf-8'))
    print(f"Mensaje enviado al Arduino suscriptor: {message}")

# Configurar el callback
client.on_message = on_message

# Conectar al broker
client.connect(mqtt_broker, mqtt_port, 60)

# Suscribirse a los tópicos
client.subscribe(topic_coord)
client.subscribe(topic_ultrasonic)

# Loop para manejar MQTT en un hilo separado
client.loop_start()

# Leer mensajes del Arduino publicador y publicarlos en MQTT
try:
    while True:
        if arduino_publisher.in_waiting > 0:
            # Leer datos del Arduino publicador
            message = arduino_publisher.readline().decode('utf-8').strip()
            # Descartar prints normales por pantalla del Arduino
            if not message.startswith("trabajo_curso"):
                continue
            print(f"Mensaje recibido del Arduino publicador: {message}")

            # Publicar en el tópico correspondiente
            client.publish(message.split("~")[0], message.split("~")[1])

except KeyboardInterrupt:
    print("Saliendo...")
finally:
    # Cerrar conexiones
    client.loop_stop()
    arduino_publisher.close()
    arduino_subscriber.close()
