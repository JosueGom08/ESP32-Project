// Filepath: src/main.cpp
#include <Arduino.h>
#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>

// --- Definición de Pines ---
#define PIN_LED_DIST 21   // LED Rojo: Indicador de Distancia del Ultrasonido
#define PIN_LED_TEMP 13   // LED Verde: Indicador de Temperatura Baja
#define PIN_TRIG 12   // Pin TRIG del sensor ultrasónico
#define PIN_ECHO 14   // Pin ECHO del sensor ultrasónico
#define PIN_DHT 15   // Pin de datos del DHT22

unsigned long tAnterior = 0; // util para utilizar millis
const char* ssid = "Wokwi-GUEST"; // Red
const char* password = ""; // contrasena

// Configuracion DHT (sensor de temperatura)
#define DHTTYPE DHT22
DHT dht(PIN_DHT, DHTTYPE);
// https://ntfy.sh/antoniog2004
// Función para medir distancia
float medirDistancia() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  
  // Mide el tiempo que tarda en regresar la onda de sonido en microsegundos
  long duracion = pulseIn(PIN_ECHO, HIGH);
  
  float cm = (duracion * 0.034) / 2.0;
  return cm;
}

void setup() {
  // Inicialización de la consola serial
  Serial.begin(115200);
  // conexion a 
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(F("."));
  }

  Serial.println(F("Conectado!!")); // se logro conectar
  

  // Configuración de pines de entrada/salida
  pinMode(PIN_LED_DIST, OUTPUT);
  pinMode(PIN_LED_TEMP, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  // Inicialización del sensor de temperatura
  dht.begin();

  // Estado inicial de los actuadores (apagados)
  digitalWrite(PIN_LED_DIST, LOW);
  digitalWrite(PIN_LED_TEMP, LOW);
}

void loop() {
  unsigned long tActual = millis();
  if(tActual - tAnterior >= 2000){
    tAnterior = tActual;
    // 1. Lectura de los Sensores
    float distancia = medirDistancia();
    float temperatura = dht.readTemperature();

    // Validación de lectura del sensor de temperatura
    if (isnan(temperatura)) {
      Serial.println(F("Error al leer el sensor DHT22!"));
      temperatura = 99.0; // Valor por defecto en caso de error
    }

    // Imprime la informacion
    Serial.print(F("Distancia: "));
    Serial.print(distancia);
    Serial.print(F(" cm | Temperatura: "));
    Serial.print(temperatura);
    Serial.println(F(" C"));

    // Si detecta algo a <= 50cm, enciende el LED Rojo
    if (distancia <= 50.0) {
      digitalWrite(PIN_LED_DIST, HIGH);
    } else {
      digitalWrite(PIN_LED_DIST, LOW);
    }

    // Si la temperatura es <= 20C, enciende el LED Verde
    if (temperatura <= 20.0) {
      digitalWrite(PIN_LED_TEMP, HIGH);
    } else {
      digitalWrite(PIN_LED_TEMP, LOW);
    }


    if(WiFi.status() == WL_CONNECTED){
      HTTPClient http;

      http.begin("https://ntfy.sh/antoniog2004");

      int codigoRespuesta1 = http.POST(String(distancia));
      int codigoRespuesta2 = http.POST(String(temperatura));

      if(codigoRespuesta1 == 200 && codigoRespuesta2 == 200){
        Serial.print(F("Enviado correctamente !"));
        Serial.println(distancia);
        Serial.println(temperatura);
      }else{
        Serial.println(F("No se pudo realizar el envio de informacion"));
      }
      http.end();
    } 
  }
 
}