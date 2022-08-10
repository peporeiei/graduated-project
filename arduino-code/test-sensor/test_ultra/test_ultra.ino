#define echoPin 53 // Echo Pin
#define trigPin 51 // Trigger Pin
#define LEDPin 13 // Onboard LED

int maximumRange = 200; // Maximum range needed
int minimumRange = 0; // Minimum range needed
long duration, distance; // Duration used to calculate distance
int relay1 = 24;
int maxlevel = 0;
void setup() {
  Serial.begin (115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(LEDPin, OUTPUT); // Use LED indicator (if required)
  pinMode(25, OUTPUT);
  digitalWrite(25,LOW);
}

void loop() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);

  //Calculate the distance (in cm) based on the speed of sound.
  distance = duration / 58.2;
//  distance= duration*0.034/2;

//  if (distance >= maximumRange || distance <= minimumRange) {
//    Serial.println("-1");  //เมื่ออยู่นอกระยะให้ใช้ Print -1
    Serial.println(distance);  //แสดงค่าระยะทาง
//    digitalWrite(relay1, LOW);
//  } else {
//    if (distance == 8) {
//      Serial.println(distance);  //แสดงค่าระยะทาง
//      Serial.print("sum : )");
//      Serial.println(maxlevel);
//      maxlevel += 1;
//      if (maxlevel >= 5) {
//        Serial.println("max :)");
//        digitalWrite(relay1, HIGH);//แสดงค่าระยะทาง
//      }
//    }
//    else {
//      Serial.println(distance);
//      digitalWrite(relay1, LOW);
//    }
//  }
delay(500);
}
