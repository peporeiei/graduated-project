#include <Arduino.h>
int WaterHigh = 53;
int WaterLow = 51;
#define INTERVAL_MESSAGE1 1000 //5วินาที
unsigned long time_1 = 0;
void print_time(unsigned long time_millis);

void setup() {
  Serial.begin(9600);
  Serial.println("Start flowless Test");
  Serial.println("-------------------");
  pinMode(WaterHigh, INPUT);
  pinMode(WaterLow, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  if (millis() - time_1 > INTERVAL_MESSAGE1) {
    // เต็มล่างบน
    if (digitalRead(WaterHigh) == LOW && digitalRead(WaterLow) == LOW) {
      Serial.println("HIGH | HIGH");
      Serial.println("-------------------");
      digitalWrite(LED_BUILTIN, HIGH);
    }
    // เต็มล่างไม่เต็มบน
    if (digitalRead(WaterHigh) == HIGH && digitalRead(WaterLow) == LOW) {
      Serial.println("LOW | HIGH");
      Serial.println("-------------------");
      digitalWrite(LED_BUILTIN, LOW);
    }
    // ไม่เต็มล่าง
    if (digitalRead(WaterHigh) == HIGH && digitalRead(WaterLow) == HIGH) {
      Serial.println("LOW | LOW");
      Serial.println("-------------------");
      digitalWrite(LED_BUILTIN, LOW);
    }
    time_1 = millis();
  }
}
