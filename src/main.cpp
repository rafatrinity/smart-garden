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


#define ECHO_PIN 7
#define TRIG_PIN 8

#define DHTPIN 2         // Pino conectado ao sensor DHT22
#define DHTTYPE DHT22    // Define o tipo do sensor DHT

Servo myservo;  // Cria uma instância do Servo
#define SERVO_PIN 6

#define TEXT_START_X 150
#define TEXT_START_Y 30
#define TEXT_GAP 30
#define GRAPH_START_X 10
#define GRAPH_START_Y 300
#define GRAPH_WIDTH 20
#define GRAPH_HEIGHT 100

const float BETA = 3950; // Coeficiente Beta do termistor
const float GAMMA = 0.7;
const float RL10 = 50;

struct Previous {
    double temperature;
    double humidity;
    double distance;
    double lightIntensity;
};

struct Current {
    double temperature;
    double humidity;
    double distance;
    double lightIntensity;
};

// Inicializando uma instância da estrutura
struct Previous previous;
struct Current current;

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
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
  tft.setFont(&FreeSerifBold12pt7b); // Define a fonte para o texto
  tft.setTextSize(1); // Define o tamanho do texto

  // Texto estático
  tft.setCursor(10, 30);
  tft.print("Temp: ");
  tft.setCursor(10, 60);
  tft.print("Humidade: ");
  tft.setCursor(10, 90);
  tft.print("Dist: ");
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


void updateText(double* previous_values[], double* current_values[], const char* names[]) {
  tft.setFont(&FreeMonoBold9pt7b); // Define a fonte para os valores
  int cursor_position = 0;

  for (int i = 0; i < 4; i++) {
    cursor_position += TEXT_GAP;
    tft.setCursor(TEXT_START_X, cursor_position);
    
    if(*current_values[i] != *previous_values[i]){
    // Clear the area where the text is printed
    tft.fillRect(TEXT_START_X, cursor_position - 20, 220, 30, ILI9341_BLACK);
    
    tft.setTextColor(ILI9341_WHITE);
    tft.print(*current_values[i]);
    tft.print(names[i]);
    }
  }
}


void drawBarGraphs() {
  drawBarGraph(GRAPH_START_X, GRAPH_START_Y, GRAPH_WIDTH, GRAPH_HEIGHT, current.distance, 400); // Exemplo para distância
  drawBarGraph(GRAPH_START_X + GRAPH_WIDTH, GRAPH_START_Y, GRAPH_WIDTH, GRAPH_HEIGHT, current.temperature + 20, 70); // Exemplo para temperatura
  drawBarGraph(GRAPH_START_X + 2 * GRAPH_WIDTH, GRAPH_START_Y, GRAPH_WIDTH, GRAPH_HEIGHT, current.humidity, 100); // Exemplo para umidade
  drawBarGraph(GRAPH_START_X + 3 * GRAPH_WIDTH, GRAPH_START_Y, GRAPH_WIDTH, GRAPH_HEIGHT, current.lightIntensity, 30000); // Exemplo para luz
}

void displayData() {
  double* previous_values[] = { &previous.temperature, &previous.humidity, &previous.distance, &previous.lightIntensity };
  double* current_values[] = { &current.temperature, &current.humidity, &current.distance, &current.lightIntensity };
  const char* names[] = { " °C", " %", " Cm", " Lux" };

  updateText(previous_values, current_values, names);
  drawBarGraphs();
}

void loop() {
  current.temperature = readTemperature();
  current.humidity = readHumidity();
  current.distance = readDistanceCM();
  current.lightIntensity = readLightIntensity();

  displayData();

  if (current.distance >= 100) {
    myservo.write(90);  // Gira o servo para 90°
  } else {
    myservo.write(0);   // Retorna o servo para 0°
  }

  previous.temperature = current.temperature;
  previous.humidity = current.humidity;
  previous.distance = current.distance;
  previous.lightIntensity = current.lightIntensity;

  delay(500);
}
