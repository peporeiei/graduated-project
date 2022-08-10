int redLed = 22;
int AnalogPinA0 = A0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {
  if (digitalRead(AnalogPinA0) == HIGH)//ไม่ถึง
  {
    Serial.print("Status :");
    Serial.println("water Low");
    digitalWrite (redLed, LOW);
//    esp8266con.write("Water : Low");
  } else if (digitalRead(AnalogPinA0) == LOW)// ถึงขั่วบน
  {
    Serial.print("Status :");
    Serial.println("water High");
    digitalWrite(redLed, HIGH);
    delay(100);
    digitalWrite(redLed, LOW);
//    esp8266con.write("Water : High");
  }
}
