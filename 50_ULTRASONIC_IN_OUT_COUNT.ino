#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LCD settings
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Change 0x27 if your I2C address differs

// Ultrasonic sensor pins
#define TRIG_A 2
#define ECHO_A 3
#define TRIG_B 4
#define ECHO_B 5

// Buzzer pin
#define BUZZER 6

// Detection settings
const int distanceThreshold = 5; // cm
const unsigned long detectionTimeout = 4000; // 4 seconds

// Variables
unsigned long startTimeA = 0;
unsigned long startTimeB = 0;
bool waitingForB = false;
bool waitingForA = false;
int inCount = 0;
int outCount = 0;
int insideCount = 0;

// Function to measure distance
long getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 30000); // timeout 30ms
  long distance = duration * 0.034 / 2;
  return distance;
}

void setup() {
  pinMode(TRIG_A, OUTPUT);
  pinMode(ECHO_A, INPUT);
  pinMode(TRIG_B, OUTPUT);
  pinMode(ECHO_B, INPUT);
  pinMode(BUZZER, OUTPUT);

  lcd.init();
  lcd.backlight();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IN:0   OUT:0");
  lcd.setCursor(0, 1);
  lcd.print("INSIDE:0");
}

void loop() {
  long distA = getDistance(TRIG_A, ECHO_A);
  long distB = getDistance(TRIG_B, ECHO_B);

  unsigned long currentTime = millis();

  // --- Detection for IN (A → B) ---
  if (!waitingForB && distA <= distanceThreshold) {
    waitingForB = true;
    startTimeA = currentTime;
  }

  if (waitingForB) {
    unsigned long elapsed = currentTime - startTimeA;

    // Optional: show countdown on LCD
    lcd.setCursor(13, 1);
    lcd.print((4 - (elapsed / 1000)) >= 0 ? (4 - (elapsed / 1000)) : 0);
    lcd.print("s");

    if (distB <= distanceThreshold && elapsed <= detectionTimeout) {
      inCount++;
      insideCount++;
      beep();
      waitingForB = false;
      updateLCD();
    } else if (elapsed > detectionTimeout) {
      waitingForB = false;
      updateLCD();
    }
  }

  // --- Detection for OUT (B → A) ---
  if (!waitingForA && distB <= distanceThreshold) {
    waitingForA = true;
    startTimeB = currentTime;
  }

  if (waitingForA) {
    unsigned long elapsed = currentTime - startTimeB;

    lcd.setCursor(13, 1);
    lcd.print((4 - (elapsed / 1000)) >= 0 ? (4 - (elapsed / 1000)) : 0);
    lcd.print("s");

    if (distA <= distanceThreshold && elapsed <= detectionTimeout) {
      outCount++;
      if (insideCount > 0) insideCount--;
      beep();
      waitingForA = false;
      updateLCD();
    } else if (elapsed > detectionTimeout) {
      waitingForA = false;
      updateLCD();
    }
  }

  delay(100);
}

void beep() {
  digitalWrite(BUZZER, HIGH);
  delay(1000);
  digitalWrite(BUZZER, LOW);
}

void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IN:");
  lcd.print(inCount);
  lcd.setCursor(7, 0);
  lcd.print("OUT:");
  lcd.print(outCount);
  lcd.setCursor(0, 1);
  lcd.print("INSIDE:");
  lcd.print(insideCount);
}
\