#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

// Credenciales WiFi
const char* ssid = "tu_SSID";
const char* password = "tu_PASSWORD";
const char* serverName = "http://<IP_DEL_SERVIDOR>:5000/insertar_datos";  // URL del servidor Flask

// Configuración de los sensores DHT
#define DHTPIN1 4  // Pin del primer DHT11
#define DHTPIN2 14 // Pin del segundo DHT11
#define DHTPIN3 27 // Pin del tercer DHT11
#define DHTTYPE DHT11
DHT dht1(DHTPIN1, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);
DHT dht3(DHTPIN3, DHTTYPE);

// Configuración del sensor ultrasónico
#define TRIGGER_PIN 12  // Pin Trigger del HC-SR04
#define ECHO_PIN 13     // Pin Echo del HC-SR04
#define ALTURA_RECIPIENTE 100  // Altura total del recipiente en cm

// Configuración de alarmas
#define UMBRAL_NIVEL_AGUA 10     // Nivel mínimo aceptable de agua en cm
#define UMBRAL_TEMP_ELEVADA 35.0 // Temperatura máxima aceptable en °C
#define HUMEDAD_BAJA 30.0        // Umbral de humedad baja (%)
#define HUMEDAD_ALTA 70.0        // Umbral de humedad alta (%)

void setup() {
  Serial.begin(115200);

  // Conexión a WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi...");
  }
  Serial.println("Conexión establecida a WiFi.");

  // Inicializar sensores DHT
  dht1.begin();
  dht2.begin();
  dht3.begin();

  // Configurar pines del sensor ultrasónico
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() {
  // Leer datos de los sensores
  float humedad1 = dht1.readHumidity();
  float temperatura1 = dht1.readTemperature();

  float humedad2 = dht2.readHumidity();
  float temperatura2 = dht2.readTemperature();

  float humedad3 = dht3.readHumidity();
  float temperatura3 = dht3.readTemperature();

  long nivelAgua = medirNivelAgua();

  // Verificar lecturas válidas
  if (isnan(humedad1) || isnan(temperatura1) || isnan(humedad2) || isnan(temperatura2) || 
      isnan(humedad3) || isnan(temperatura3)) {
    Serial.println("Error al leer de los sensores DHT.");
    delay(2000);
    return;
  }

  // Calcular promedio de humedad
  float promedioHumedad = (humedad1 + humedad2 + humedad3) / 3;

  // Verificar alarmas
  bool alarmaNivelAgua = (nivelAgua < UMBRAL_NIVEL_AGUA);
  bool alarmaTempElevada = (temperatura1 > UMBRAL_TEMP_ELEVADA || 
                            temperatura2 > UMBRAL_TEMP_ELEVADA || 
                            temperatura3 > UMBRAL_TEMP_ELEVADA);
  bool alarmaHumedadBaja = (promedioHumedad < HUMEDAD_BAJA);
  bool alarmaHumedadAlta = (promedioHumedad > HUMEDAD_ALTA);

  // Imprimir alarmas en el monitor serie
  if (alarmaNivelAgua) Serial.println("ALERTA: Nivel de agua bajo.");
  if (alarmaTempElevada) Serial.println("ALERTA: Temperatura elevada.");
  if (alarmaHumedadBaja) Serial.println("ALERTA: Humedad baja.");
  if (alarmaHumedadAlta) Serial.println("ALERTA: Humedad alta.");

  // Crear el JSON con los datos y las alarmas
  String jsonData = "{\"humedad1\": " + String(humedad1) + 
                    ", \"temperatura1\": " + String(temperatura1) + 
                    ", \"humedad2\": " + String(humedad2) + 
                    ", \"temperatura2\": " + String(temperatura2) + 
                    ", \"humedad3\": " + String(humedad3) + 
                    ", \"temperatura3\": " + String(temperatura3) + 
                    ", \"nivelAgua\": " + String(nivelAgua) + 
                    ", \"alarmaNivelAgua\": " + String(alarmaNivelAgua) + 
                    ", \"alarmaTempElevada\": " + String(alarmaTempElevada) + 
                    ", \"alarmaHumedadBaja\": " + String(alarmaHumedadBaja) + 
                    ", \"alarmaHumedadAlta\": " + String(alarmaHumedadAlta) + "}";

  // Enviar los datos al servidor
  HTTPClient http;
  http.begin(serverName);
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST(jsonData);
  if (httpResponseCode > 0) {
    Serial.println("Datos enviados correctamente.");
    Serial.println("Código de respuesta HTTP: " + String(httpResponseCode));
  } else {
    Serial.println("Error al enviar los datos.");
    Serial.println("Código de error HTTP: " + String(httpResponseCode));
  }

  // Cerrar la conexión
  http.end();

  // Esperar 10 segundos antes de la próxima iteración
  delay(10000);
}

// Función para medir la distancia usando el HC-SR04
long medirDistancia() {
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  long duracion = pulseIn(ECHO_PIN, HIGH);
  long distancia = (duracion / 2) / 29.1; // Velocidad del sonido en aire
  return distancia;
}

// Función para medir el nivel del agua
long medirNivelAgua() {
  long distancia = medirDistancia();
  long nivelAgua = ALTURA_RECIPIENTE - distancia;
  if (nivelAgua < 0) nivelAgua = 0;
  return nivelAgua;
}
