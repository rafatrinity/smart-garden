#include <Wire.h>
#include "DHT.h"
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <Servo.h>
#include <Fonts/FreeSerifBold12pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <math.h>

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

Servo myservo;  // Cria uma instância do Servo
#define SERVO_PIN 6

// const uint8_t sun_icon [] PROGMEM = {
//   0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x7E,
//   0x18, 0x7E, 0x00, 0x00, 0x18, 0x18, 0xFF, 0x18,
//   0x18, 0x00, 0x7E, 0x18, 0x7E, 0x00, 0x00, 0x00,
//   0x18, 0x00, 0x00, 0x00, 0x00
// };

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  dht.begin(); // Inicializa o sensor DHT

  tft.begin(); // Inicializa o display ILI9341
  tft.setRotation(3); // Ajusta a rotação do display, ajuste conforme necessário
  tft.fillScreen(ILI9341_BLACK); // Preenche a tela com preto
  tft.setTextColor(ILI9341_WHITE); // Define a cor do texto como branco
  tft.setFont(&FreeSerifBold12pt7b); // Define a fonte para o texto
  tft.setTextSize(1); // Define o tamanho do texto

  // Texto estático
  tft.setCursor(10, 30);
  tft.print("Distancia: ");
  tft.setCursor(10, 60);
  tft.print("Temp: ");
  tft.setCursor(10, 90);
  tft.print("Humidade: ");
  tft.setCursor(10, 120);
  tft.print("Luz: ");
  
  myservo.attach(SERVO_PIN);  // Conecta o servo ao pino definido
  myservo.write(0);  // Inicializa o servo na posição 0°
}

float readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  int duration = pulseIn(ECHO_PIN, HIGH);
  return duration * 0.0342 / 2;
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

void drawBarGraph(int x, int y, int width, int height, float value, float max_value) {
  value = log(value);
  max_value = log(max_value);

  // Limpa a área do gráfico antes de desenhar
  tft.fillRect(x, y - height, width, height, ILI9341_BLACK);
  
  // Garante que o valor não ultrapasse o valor máximo
  if (value > max_value) {
    value = max_value;
  }
  
  // Calcula a altura da barra proporcional ao valor
  float barHeight = ((value / max_value) * height);
  
  // Desenha o contorno da barra
  tft.drawRect(x, y - height, width, height, ILI9341_WHITE);
  
  // Preenche a barra com a cor verde
  tft.fillRect(x + 1, y - barHeight, width - 2, barHeight, ILI9341_GREEN);
}

void displayData(float distance, float temperature, float humidity, int lightIntensity) {
  tft.setFont(&FreeMonoBold9pt7b); // Define a fonte para os valores
  tft.setTextColor(ILI9341_WHITE); // Define a cor do texto como branco para escrever o texto novo

  // Exibe ícones junto com os valores
  // tft.drawBitmap(10, 10, sun_icon, 16, 16, ILI9341_YELLOW);

  clearTextArea(150, 10, 220, 30);
  tft.setCursor(150, 30); // Define a posição do cursor para o valor da distância
  tft.print(distance);
  tft.print(" cm");

  clearTextArea(150, 40, 220, 30);
  tft.setCursor(150, 60); // Define a posição do cursor para o valor da temperatura
  tft.print(temperature);
  tft.print(" C");

  clearTextArea(150, 70, 220, 30);
  tft.setCursor(150, 90); // Define a posição do cursor para o valor da humidade
  tft.print(humidity);
  tft.print(" %");

  clearTextArea(150, 100, 220, 30);
  tft.setCursor(150, 120); // Define a posição do cursor para o valor da luz
  tft.print(lightIntensity);
  tft.print(" lux");

  // Desenha gráficos de barras
  drawBarGraph(10, 300, 20, 100, distance, 400); // Exemplo para distância
  drawBarGraph(40, 300, 20, 100, temperature + 20, 70); // Exemplo para temperatura
  drawBarGraph(70, 300, 20, 100, humidity, 100); // Exemplo para umidade
  drawBarGraph(100, 300, 20, 100, lightIntensity, 30000); // Exemplo para luz
}

void loop() {
  float distance = readDistanceCM();
  float temperature = readTemperature();
  float humidity = readHumidity();
  int lightIntensity = readLightIntensity();

  displayData(distance, temperature, humidity, lightIntensity);

  if (distance >= 100) {
    myservo.write(90);  // Gira o servo para 90°
  } else {
    myservo.write(0);   // Retorna o servo para 0°
  }

  delay(500);
}
