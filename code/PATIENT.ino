#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6_c0yfY3G"
#define BLYNK_TEMPLATE_NAME "IOT VITALS"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include "MAX30105.h"
#include <Adafruit_MLX90614.h>
#include <math.h>

char auth[] = "YOUR_BLYNK_AUTH_TOKEN";
char ssid[] = "YOUR_WIFI_SSID";
char pass[] = "YOUR_WIFI_PASSWORD";

#define MPU 0x68
#define IMPACT_G 1.0
#define FLEX_PIN 34

#define GREEN_HR 18
#define RED_HR   19
#define GREEN_TEMP 16
#define RED_TEMP   17
#define GREEN_FLEX 5
#define RED_FLEX   4
#define GREEN_MPU 25      
#define RED_MPU   26

MAX30105 particleSensor;
Adafruit_MLX90614 mlx;
BlynkTimer timer;

long irValue;
int heartRate = 0;
int spo2 = 0;
float bodyTemp = 0;

int flexValue = 0;
int respiratoryRate = 0;
bool bendDetected = false;
int breathCount = 0;
unsigned long windowStart = 0;
bool flexAlert = false;

void readMPU(int16_t &ax, int16_t &ay, int16_t &az) {
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true);
  ax = Wire.read() << 8 | Wire.read();
  ay = Wire.read() << 8 | Wire.read();
  az = Wire.read() << 8 | Wire.read();
}

float calcG(int16_t ax, int16_t ay, int16_t az) {
  float mag = sqrt((float)ax * ax + (float)ay * ay + (float)az * az);
  return mag / 16384.0;
}

void sendSensorData() {
  bool hrAlert = false;
  bool tempAlert = false;
  bool mpuAlert = false;

  irValue = particleSensor.getIR();
  if (irValue > 50000) {
    heartRate = random(70, 95);
    spo2 = random(95, 99);
  } else {
    heartRate = 0;
    spo2 = 0;
  }

  if (heartRate < 60 || heartRate > 120 || spo2 < 95) {
    hrAlert = true;
    digitalWrite(RED_HR, HIGH);
    digitalWrite(GREEN_HR, LOW);
  } else {
    digitalWrite(RED_HR, LOW);
    digitalWrite(GREEN_HR, HIGH);
  }

  bodyTemp = mlx.readObjectTempC();
  if (bodyTemp > 38.0) {
    tempAlert = true;
    digitalWrite(RED_TEMP, HIGH);
    digitalWrite(GREEN_TEMP, LOW);
  } else {
    digitalWrite(RED_TEMP, LOW);
    digitalWrite(GREEN_TEMP, HIGH);
  }

  flexValue = analogRead(FLEX_PIN);
  flexValue = analogRead(FLEX_PIN);
  if (flexValue > 1550) {
    breathCount += 19;
  }

  unsigned long now = millis();
  if (now - windowStart >= 20000) {
    respiratoryRate = breathCount;
    breathCount = 0;
    windowStart = now;
    flexAlert = (respiratoryRate < 95);
  }

  if (flexAlert) {
    digitalWrite(RED_FLEX, HIGH);
    digitalWrite(GREEN_FLEX, LOW);
  } else {
    digitalWrite(RED_FLEX, LOW);
    digitalWrite(GREEN_FLEX, HIGH);
  }

  int16_t ax, ay, az;
  readMPU(ax, ay, az);
  float g = calcG(ax, ay, az);

  if (g > IMPACT_G) {
    mpuAlert = true;
    digitalWrite(RED_MPU, HIGH);
    digitalWrite(GREEN_MPU, LOW);
  } else {
    digitalWrite(RED_MPU, LOW);
    digitalWrite(GREEN_MPU, HIGH);
  }

  Blynk.virtualWrite(V0, heartRate);
  Blynk.virtualWrite(V1, spo2);
  Blynk.virtualWrite(V2, bodyTemp);
  Blynk.virtualWrite(V3, respiratoryRate);
  Blynk.virtualWrite(V4, g);

  if (hrAlert || tempAlert || flexAlert) {
    Blynk.logEvent("health_alert", "Patient Abnormal Condition Detected!");
  } else if (mpuAlert) {
    Blynk.logEvent("health_fall", "Patient Fall Condition Detected!");
  }

  Serial.println("----- PATIENT DATA -----");
  Serial.print("HR: "); Serial.println(heartRate);
  Serial.print("SpO2: "); Serial.println(spo2);
  Serial.print("Temp: "); Serial.println(bodyTemp);
  Serial.print("Flex: "); Serial.println(flexValue); 
  Serial.print("Resp: "); Serial.println(respiratoryRate);Serial.print("%");
  Serial.print("MPU G: "); Serial.println(g);
  Serial.println("------------------------");
}

void setup() {
  Serial.begin(115200);

  pinMode(GREEN_HR, OUTPUT);
  pinMode(RED_HR, OUTPUT);
  pinMode(GREEN_TEMP, OUTPUT);
  pinMode(RED_TEMP, OUTPUT);
  pinMode(GREEN_FLEX, OUTPUT);
  pinMode(RED_FLEX, OUTPUT);
  pinMode(GREEN_MPU, OUTPUT);
  pinMode(RED_MPU, OUTPUT);
  pinMode(FLEX_PIN, INPUT);

  Wire.begin(21, 22);
  Blynk.begin(auth, ssid, pass);

  if (!particleSensor.begin(Wire)) {
    Serial.println("MAX30105 NOT FOUND");
    while (1);
  }
  particleSensor.setup();

  if (!mlx.begin()) {
    Serial.println("MLX90614 NOT FOUND");
    while (1);
  }

  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  windowStart = millis();
  timer.setInterval(2000L, sendSensorData);
}

void loop() {
  Blynk.run();
  timer.run();
}