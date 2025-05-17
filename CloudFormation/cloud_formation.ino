#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <DHT.h>

// TFT Display Configuration
#define TFT_CS     53
#define TFT_RST    48
#define TFT_DC     49
#define TFT_SCLK   52
#define TFT_MOSI   51
#define TFT_BL     12
Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// Sensor Configuration
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP280 bmp;
#define MQ2_PIN A0      // Butane gas sensor analog pin
#define LED_PIN 10      // LED indicator pin
#define BUTANE_WARNING_LEVEL 300   // Threshold for butane warning

// Relay Control Pins (Active LOW)
#define FAN_PIN 5
#define PUMP_PIN 6
#define PELTIER_PIN 3
#define HUMIDIFIER_PIN 4

// Cloud Formation Parameters
#define HUMIDIFY_TARGET 85.0       // Target RH before cooling
#define COOLING_TEMP 16.0          // Minimum cooling temp for visible condensation
#define HUMIDIFY_DURATION 20000    // 20 seconds
#define MIN_PRESSURE 800.0         // Minimum safe pressure

// System State
enum CloudPhase { IDLE, HUMIDIFYING, COOLING };
CloudPhase currentPhase = IDLE;
unsigned long phaseStartTime = 0;

// Function declarations
void showError(const char* msg);
void allDevicesOff();
void controlCloudProcess(float h, float t, float p);
void startHumidifying();
void startCooling();
void abortProcess();
void drawInterface();
void updateDisplay(float h, float t, float p, int butane);
void updateDeviceStatus(int x, int y, int pin);
float calculateDewPoint(float t, float h);

void setup() {
  Serial.begin(9600);

  // Initialize Display
  pinMode(TFT_BL, OUTPUT);
  analogWrite(TFT_BL, 255);
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(0);
  tft.fillScreen(ST7735_BLACK);

  // Initialize Sensors
  dht.begin();
  if (!bmp.begin(0x76)) {
    showError("BMP280 Error!");
    while(1);
  }
  pinMode(MQ2_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);  // Initialize LED pin

  // Initialize Relays (Active LOW)
  pinMode(FAN_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(PELTIER_PIN, OUTPUT);
  pinMode(HUMIDIFIER_PIN, OUTPUT);
  allDevicesOff();

  drawInterface();
}

void loop() {
  // Read Sensor Data
  float humidity = dht.readHumidity();
  float temperature = bmp.readTemperature();
  float pressure = bmp.readPressure() / 100.0F;
  int butaneLevel = analogRead(MQ2_PIN);  // Read butane level

  if (isnan(humidity)) {
    showError("DHT22 Error!");
    return;
  }

  // Control LED based on butane level
  if (butaneLevel > BUTANE_WARNING_LEVEL) {
    digitalWrite(LED_PIN, HIGH);  // Turn on LED if butane level is high
  } else {
    digitalWrite(LED_PIN, LOW);   // Turn off LED if safe
  }

  // Control Logic
  controlCloudProcess(humidity, temperature, pressure);

  // Update Display with butane reading
  updateDisplay(humidity, temperature, pressure, butaneLevel);

  delay(1000);
}

// Dew Point Formula
float calculateDewPoint(float t, float h) {
  return t - ((100.0 - h) / 5.0);
}

void controlCloudProcess(float h, float t, float p) {
  float dewPoint = calculateDewPoint(t, h);

  switch(currentPhase) {
    case IDLE:
      if (h < HUMIDIFY_TARGET && t >= 22.0 && p >= MIN_PRESSURE) {
        startHumidifying();
      }
      break;

    case HUMIDIFYING:
      if (millis() - phaseStartTime >= HUMIDIFY_DURATION) {
        if (h >= HUMIDIFY_TARGET) {
          startCooling();
        } else {
          abortProcess();
        }
      }
      break;

    case COOLING:
      if (t <= dewPoint || (t <= COOLING_TEMP && h >= HUMIDIFY_TARGET)) {
        // Clouds forming — keep running
      } else if (t < 12.0 || h > 99.0) {
        abortProcess(); // avoid overcooling or saturation
      }
      break;
  }
}

void startHumidifying() {
  currentPhase = HUMIDIFYING;
  phaseStartTime = millis();
  digitalWrite(HUMIDIFIER_PIN, LOW); // ON
  digitalWrite(FAN_PIN, LOW);        // ON
}

void startCooling() {
  currentPhase = COOLING;
  phaseStartTime = millis();
  digitalWrite(PELTIER_PIN, LOW);    // ON
  digitalWrite(PUMP_PIN, LOW);       // ON
  // Keep fan running
}

void abortProcess() {
  currentPhase = IDLE;
  allDevicesOff();
}

void allDevicesOff() {
  digitalWrite(FAN_PIN, HIGH);
  digitalWrite(PUMP_PIN, HIGH);
  digitalWrite(PELTIER_PIN, HIGH);
  digitalWrite(HUMIDIFIER_PIN, HIGH);
}

void drawInterface() {
  tft.fillScreen(ST7735_BLACK);

  // Sensor Labels
  tft.setTextColor(ST7735_WHITE);
  tft.setCursor(10, 10);  tft.print("Humidity:");
  tft.setCursor(10, 30);  tft.print("Temp:");
  tft.setCursor(10, 50);  tft.print("Pressure:");
  tft.setCursor(10, 70);  tft.print("Butane:");
  tft.setCursor(10, 90);  tft.print("Phase:");

  // Device Status Labels
  tft.setCursor(10, 110); tft.print("Fan:");
  tft.setCursor(10, 130); tft.print("Humidifier:");
  tft.setCursor(10, 150); tft.print("Peltier:");
  tft.setCursor(10, 170); tft.print("Pump:");
}

void updateDisplay(float h, float t, float p, int butane) {
  // Update Sensor Values
  tft.fillRect(80, 10, 60, 8, ST7735_BLACK);
  tft.setTextColor(ST7735_CYAN);
  tft.setCursor(80, 10); tft.print(h, 1); tft.print("%");

  tft.fillRect(80, 30, 60, 8, ST7735_BLACK);
  tft.setTextColor(ST7735_GREEN);
  tft.setCursor(80, 30); tft.print(t, 1); tft.print("C");

  tft.fillRect(80, 50, 60, 8, ST7735_BLACK);
  tft.setTextColor(ST7735_YELLOW);
  tft.setCursor(80, 50); tft.print(p, 1); tft.print("hPa");

  // Butane reading
  tft.fillRect(80, 70, 60, 8, ST7735_BLACK);
  tft.setTextColor(ST7735_WHITE);
  tft.setCursor(80, 70); tft.print(butane);

  // Update Phase
  tft.fillRect(80, 90, 60, 8, ST7735_BLACK);
  tft.setTextColor(ST7735_MAGENTA);
  tft.setCursor(80, 90);
  switch(currentPhase) {
    case IDLE: tft.print("IDLE"); break;
    case HUMIDIFYING: tft.print("HUMIDIFY"); break;
    case COOLING: tft.print("COOLING"); break;
  }

  // Update Device Status (Green=ON, Red=OFF)
  updateDeviceStatus(80, 110, FAN_PIN);
  updateDeviceStatus(80, 130, HUMIDIFIER_PIN);
  updateDeviceStatus(80, 150, PELTIER_PIN);
  updateDeviceStatus(80, 170, PUMP_PIN);
}

void updateDeviceStatus(int x, int y, int pin) {
  tft.fillRect(x, y, 30, 8, ST7735_BLACK);
  if (digitalRead(pin) == LOW) {
    tft.setTextColor(ST7735_GREEN);
    tft.setCursor(x, y); tft.print("ON");
  } else {
    tft.setTextColor(ST7735_RED);
    tft.setCursor(x, y); tft.print("OFF");
  }
}

void showError(const char* msg) {
  tft.fillScreen(ST7735_BLACK);
  tft.setTextColor(ST7735_RED);
  tft.setCursor(10, 40);
  tft.print(msg);
  Serial.println(msg);
}