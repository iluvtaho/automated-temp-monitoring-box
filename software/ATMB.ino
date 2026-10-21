/*
  ============================================
  Motion-Activated Temperature Monitor
  ============================================
  Design & Implementation: Jared Caldo
  Date: 2026
  ============================================
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <DHT.h>

// ---------- Pin assignments ----------
const int PIR_PIN   = 6;   // HC-SR501 OUT
const int SERVO_PIN = 9;   // Micro servo signal (needs a PWM pin)
const int DHT_PIN   = 7;   // DHT11 data pin
#define DHT_TYPE DHT11
const int BUZZER_PIN = 4;   // passive buzzer signal pin
const int BUZZ_FREQ  = 2000; // tone frequency in Hz
const int BUZZ_MS    = 3000; 

// ---------- Sweep configuration ----------
const int SERVO_LEFT  = 0;    // degrees
const int SERVO_RIGHT = 180;  // degrees
const int SWEEP_COUNT = 3;    // number of left->right->left cycles
const int STEP_DELAY  = 15;   // ms between each 1-degree step (sweep speed)

// ---------- Objects ----------
LiquidCrystal_I2C lcd(0x27, 16, 2); 
Servo myServo;
DHT dht(DHT_PIN, DHT_TYPE);

void setup() {
  pinMode(PIR_PIN, INPUT);
  myServo.attach(SERVO_PIN);
  myServo.write(SERVO_LEFT);   // rest position

  lcd.init();
  lcd.backlight();
  lcd.init();
  lcd.backlight();
  
  // Credit splash screen
  lcd.setCursor(1, 0);
  lcd.print("Design & Code");
  delay(1000);
  lcd.setCursor(5, 1);
  lcd.print("by JC");
  delay(2000);
  lcd.clear();
  
  dht.begin();

  showIdleMessage();
}

void loop() {
  if (digitalRead(PIR_PIN) == HIGH) {
    float sum = 0;
    int validReadings = 0;

    for (int i = 0; i < SWEEP_COUNT; i++) {
      float t = sweepOnce();   // sweeps once, returns that reading
      if (!isnan(t)) {
        sum += t;
        validReadings++;
      }
    }
    myServo.write(SERVO_LEFT);   // return to rest

    if (validReadings > 0) {
      displayAverage(sum / validReadings);
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Sensor error");
    }

    // Wait for motion to clear before re-arming. The average stays on the LCD the whole time and is only replaced the next time motion is detected and a new average is computed.
    while (digitalRead(PIR_PIN) == HIGH) {
      delay(200);
    }
  }
}

// "bzzt, (3s silence), buzz" - repeated for 2 cycles
void playBuzzPattern() {
  for (int cycle = 0; cycle < 3; cycle++) {
    tone(BUZZER_PIN, BUZZ_FREQ, BUZZ_MS);
    delay(BUZZ_MS);
    delay(3000);
    noTone(BUZZER_PIN);
  }
}

// One left->right->left cycle. Reads the DHT11 at the rightmost point and returns that reading (does NOT touch the LCD).
float sweepOnce() {
  for (int pos = SERVO_LEFT; pos <= SERVO_RIGHT; pos++) {
    myServo.write(pos);
    delay(STEP_DELAY);
  }

  float temp = dht.readTemperature();  // Celsius

  for (int pos = SERVO_RIGHT; pos >= SERVO_LEFT; pos--) {
    myServo.write(pos);
    delay(STEP_DELAY);
  }

  return temp;
}

void displayAverage(float avgTemp) {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Avg: ");
  lcd.print(avgTemp, 1);
  lcd.print((char)223);  // degree symbol
  lcd.print("C");

  lcd.setCursor(0, 1);
  if (avgTemp < 20) {
    lcd.print("COLD-Set higher");
  } else if (avgTemp > 25) {
    lcd.print("HOT-Set lower");
  } else {
    lcd.print("WARM-OK");
  }
  playBuzzPattern();
}
void showIdleMessage() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waiting for");
  lcd.setCursor(0, 1);
  lcd.print("motion...");
}
