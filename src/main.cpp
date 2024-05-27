#include <Wire.h>
#include "DHT.h"
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

#define TFT_DC 9
#define TFT_CS 10

#define WIDTH 240
#define HEIGHT 320

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

#define ECHO_PIN 7
#define TRIG_PIN 8

const float BETA = 3950; // Coeficiente Beta do termistor
const float GAMMA = 0.7;
const float RL10 = 50;

#define DHTPIN 2         // Pino conectado ao sensor DHT22
#define DHTTYPE DHT22    // Define o tipo do sensor DHT

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  dht.begin(); // Inicializa o sensor DHT

  tft.begin(); // Inicializa o display ILI9341
  tft.setRotation(3); // Ajusta a rotação do display, ajuste conforme necessário
  tft.fillScreen(ILI9341_BLACK); // Preenche a tela com preto
  tft.setTextColor(ILI9341_WHITE); // Define a cor do texto como branco
  tft.setTextSize(2); // Define o tamanho do texto

  // Texto estático
  tft.setCursor(10, 10);
  tft.print("Distancia: ");
  tft.setCursor(10, 40);
  tft.print("Temp: ");
  tft.setCursor(10, 70);
  tft.print("Humidade: ");
  tft.setCursor(10, 100);
  tft.print("Luz: ");
}

float readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  int duration = pulseIn(ECHO_PIN, HIGH);
  return duration * 0.034 / 2;
}

float readTemperature() {
  float t = dht.readTemperature();
  if (isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return -999.0; // Valor de erro
  }
  return t;
}

float readHumidity() {
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return -999.0; // Valor de erro
  }
  return h;
}

int readLightIntensity() {
  int analogValue = analogRead(A0);
  float voltage = analogValue / 1024.0 * 5.0;
  float resistance = 2000.0 * voltage / (1.0 - voltage / 5.0);
  float lux = pow(RL10 * 1e3 * pow(10, GAMMA) / resistance, (1 / GAMMA));
  return lux;
}

void clearTextArea(int x, int y, int w, int h) {
  tft.fillRect(x, y, w, h, ILI9341_BLACK);
}

void displayData(float distance, float temperature, float humidity, int lightIntensity) {
  tft.setTextColor(ILI9341_WHITE); // Define a cor do texto como branco para escrever o texto novo
  clearTextArea(150, 10, 220, 30);
  tft.setCursor(150, 10); // Define a posição do cursor para o valor da distância
  tft.print(distance);
  tft.print(" cm");

  clearTextArea(150, 40, 220, 30);
  tft.setCursor(150, 40); // Define a posição do cursor para o valor da temperatura
  tft.print(temperature);
  tft.print(" C");

  clearTextArea(150, 70, 220, 30);
  tft.setCursor(150, 70); // Define a posição do cursor para o valor da humidade
  tft.print(humidity);
  tft.print(" %");

  clearTextArea(150, 100, 220, 30);
  tft.setCursor(150, 100); // Define a posição do cursor para o valor da luz
  tft.print(lightIntensity);
  tft.print(" lux");
}

void loop() {
  float distance = readDistanceCM();
  float temperature = readTemperature();
  float humidity = readHumidity();
  int lightIntensity = readLightIntensity();

  displayData(distance, temperature, humidity, lightIntensity);

  delay(100);
}
