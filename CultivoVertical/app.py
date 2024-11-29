from flask import Flask, request, jsonify
from flask_socketio import SocketIO
import mysql.connector
import logging

app = Flask(__name__)
socketio = SocketIO(app)  # Inicializamos SocketIO para WebSockets

# Configuración para la conexión a la base de datos
def conectar_base_datos():
    try:
        conn = mysql.connector.connect(
            host="localhost",
            user="tu_usuario",
            password="tu_contraseña",
            database="CultivoVertical"
        )
        return conn
    except mysql.connector.Error as e:
        logging.error(f"Error al conectar a la base de datos: {e}")
        return None

@app.route('/insertar_datos', methods=['POST'])
def insertar_datos():
    try:
        # Leer datos del JSON enviado por la ESP32
        data = request.json
        humedad = data.get('humedad')
        temperatura = data.get('temperatura')
        nivelAgua = data.get('nivelAgua')
        
        # Alarmas que la ESP32 ha enviado
        alarmas = {
            "NivelAguaBajo": data.get('NivelAguaBajo'),
            "TemperaturaElevada": data.get('TemperaturaElevada'),
            "HumedadBaja": data.get('HumedadBaja'),
            "HumedadAlta": data.get('HumedadAlta')
        }

        # Conectar a la base de datos
        conn = conectar_base_datos()
        if conn is None:
            return jsonify({"error": "No se pudo conectar a la base de datos"}), 500

        cursor = conn.cursor()

        # Insertar datos en las tablas correspondientes
        if humedad is not None:
            cursor.execute("INSERT INTO Humedad (Humedad) VALUES (%s)", (humedad,))
        if temperatura is not None:
            cursor.execute("INSERT INTO Temperatura (Temperatura) VALUES (%s)", (temperatura,))
        if nivelAgua is not None:
            cursor.execute("INSERT INTO UltraSonico (Distancia) VALUES (%s)", (nivelAgua,))

        # Insertar las alarmas en la tabla de alarmas
        cursor.execute(""" 
            INSERT INTO Alarmas (NivelAguaBajo, TemperaturaElevada, HumedadBaja, HumedadAlta) 
            VALUES (%s, %s, %s, %s)
        """, (alarmas["NivelAguaBajo"], alarmas["TemperaturaElevada"], alarmas["HumedadBaja"], alarmas["HumedadAlta"]))

        conn.commit()

        # Emitir los datos y las alarmas activadas al frontend
        socketio.emit('nuevos_datos', {
            "humedad": humedad,
            "temperatura": temperatura,
            "nivelAgua": nivelAgua,
            "alarmas": alarmas  # Enviar las alarmas activadas
        })

        # Cerrar la conexión
        cursor.close()
        conn.close()

        return jsonify({"message": "Datos y alarmas procesados correctamente"}), 200
    except Exception as e:
        print(f"Error al procesar los datos: {e}")
        return jsonify({"error": "Ocurrió un error al procesar los datos"}), 500

if __name__ == '__main__':
    socketio.run(app, debug=True, host='0.0.0.0', port=5000)
