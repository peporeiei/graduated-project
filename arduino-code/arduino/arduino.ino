#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Arduino.h>
/**
   RX = 11
   TX = 10
*/
SoftwareSerial esp8266con(11, 10);
int redLed = 22;
int AnalogPinA0 = A0;
int relay1 = 31;
int relay2 = 33;
int relay3 = 35;
int relay4 = 37;
int solinoid = 25;
int system_status = 23;
////////////////////////////////////// ultrasonic && test level
#define echoPin 53 // Echo Pin
#define trigPin 51 // Trigger Pin
long duration;
long distance; // Duration used to calculate distance
int maxlevel = 1;
int lowlevel = 1;
String value_callback = "";
///////////////////////////////////// vr sensor
#define doPin A1
////////////////////////////////////// millis
#define INTERVAL_MESSAGE1 1000 //1นาที
#define INTERVAL_MESSAGE2 60000 //1นาที
unsigned long time_1 = 0;
unsigned long time_2 = 0;
unsigned long time_stop = 0;
void print_time(unsigned long time_millis);
String poor_name = "", poor_id = "", mode_working = "";
int pump_port = 0;
boolean checkmanaul = false;
boolean check_interupt = false;
boolean check_reset = false;
/////////////// pH Sensor Config
#define SensorPin A15 //pH meter Analog output to Arduino Analog Input 0
#define Offset 0.00 //deviation compensate
unsigned long int avgValue; //Store the average value of the sensor feedback
//////////////// Temp sensor
#define ONE_WIRE_BUS 49
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
/////////////// Do sensor
#define DO_PIN A13
#define VREF 5000    //VREF (mv)
#define ADC_RES 1024 //ADC Resolution
#define TWO_POINT_CALIBRATION 0
//Single point calibration needs to be filled CAL1_V and CAL1_T
#define CAL1_V (131) //mv
#define CAL1_T (25)   //℃
//Two-point calibration needs to be filled CAL2_V and CAL2_T
//CAL1 High temperature point, CAL2 Low temperature point
#define CAL2_V (1300) //mv
#define CAL2_T (15)   //℃
const uint16_t DO_Table[41] = {
  14460, 14220, 13820, 13440, 13090, 12740, 12420, 12110, 11810, 11530,
  11260, 11010, 10770, 10530, 10300, 10080, 9860, 9660, 9460, 9270,
  9080, 8900, 8730, 8570, 8410, 8250, 8110, 7960, 7820, 7690,
  7560, 7430, 7300, 7180, 7070, 6950, 6840, 6730, 6630, 6530, 6410
};
uint8_t Temperaturet;
uint16_t ADC_Raw;
uint16_t ADC_Voltage;
uint16_t DO;
int16_t readDO(uint32_t voltage_mv, uint8_t temperature_c)
{
#if TWO_POINT_CALIBRATION == 00
  uint16_t V_saturation = (uint32_t)CAL1_V + (uint32_t)35 * temperature_c - (uint32_t)CAL1_T * 35;
  return (voltage_mv * DO_Table[temperature_c] / V_saturation);
#else
  uint16_t V_saturation = (int16_t)((int8_t)temperature_c - CAL2_T) * ((uint16_t)CAL1_V - CAL2_V) / ((uint8_t)CAL1_T - CAL2_T) + CAL2_V;
  return (voltage_mv * DO_Table[temperature_c] / V_saturation);
#endif
}

#define INTERVAL_DO 90000
unsigned long time_DO = 0;

void setup()  {
  Serial.begin(115200);
  esp8266con.begin(57600);
  Serial.println("------ Start water control ---------");
  pinMode(AnalogPinA0, INPUT);
  pinMode(redLed, OUTPUT);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);
  pinMode(solinoid, OUTPUT);
  pinMode(system_status, OUTPUT);
  digitalWrite(relay1, LOW);
  digitalWrite(relay2, LOW);
  digitalWrite(relay3, LOW);
  digitalWrite(relay4, LOW);
  digitalWrite(solinoid, LOW);
  digitalWrite(system_status, LOW);
  ///////////////
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  ///////////////
  //  attachInterrupt(5, interruptf, RISING);
  //  attachInterrupt(4, stopf, RISING);
  //////////////
  sensors.begin();
}
void loop() {
  //รับค่า serial จาก 8266
  ////////////////////////////////////////////////
  ReciveSerial();
  /////////////////////////////////////////
  /**
     เช็คว่าถ้ามีค่าเข้ามาจาก Serial monitor ให้วนรับค่าทีละตัวอักษร
     และส่งออกไปยัง esp8266 ทีละตัวอักษร
     และส่งออก Serial monitor ทีละตัวอักษร จนครบทุกตัวอักษร
     และออกจาก loop
  */
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    esp8266con.write(inChar);
    Serial.write(inChar);
  }
}

void ReciveSerial() {
  String value_serial = "";
  boolean poor1 = false;
  boolean start_1 = false;
  boolean check_water1 = false;
  check_reset = false;
  esp8266con.listen();
  while (esp8266con.available() > 0) {
    char inByte = esp8266con.read();
    value_serial += String(inByte);
  }
  if (value_serial != "") {             // test ระดับน้ำ
    Serial.println(value_serial);
    StaticJsonDocument<200> doc_payload;
    DeserializationError error = deserializeJson(doc_payload, value_serial);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }
    delay(3000);
    int pump_ports = doc_payload["pump_port"];
    const char* modes = doc_payload["mode"];
    Serial.println("------------------------------------------");
    Serial.print("modes : ");
    Serial.println(modes);
    Serial.print("pump port : ");
    Serial.println(pump_ports);
    Serial.println("------------------------------------------");
    pump_port = pump_ports;
    mode_working = String(modes);

    if (mode_working == "auto") {
      Serial.print("if mode :");
      Serial.println(mode_working);
      // เช็ค boolean
      if (start_1 == false) {
        Serial.println("start_auto_mode");
        clean_water_in();
        start_1 = true;
      }
    }
    else if (mode_working == "manaul") {
      Serial.print("if mode manaul : ");
      Serial.println(mode_working);
      Serial.println("------------------------------------------");
      Serial.print("pump port : ");
      Serial.println(pump_port);
      Serial.println("------------------------------------------");
      Serial.println("CALL FUNCTION : Clean Water");
      clean_water_in();
    }
    else if (mode_working == "stop") {
      stopf();
    }
    else if (mode_working == "interupt") {
      Serial.println("interupt mode :");
      ////// ดูดออก
      Serial.println("------------------------------------------");
      lowlevel = 1;
      while (lowlevel <= 5 && lowlevel >= 1) {
        if (millis() - time_stop > 1000) {
          digitalWrite(trigPin, LOW);
          delayMicroseconds(2);
          digitalWrite(trigPin, HIGH);
          delayMicroseconds(10);
          digitalWrite(trigPin, LOW);
          duration = pulseIn(echoPin, HIGH);
          distance = duration / 58.2;
          ////////////////////////////
          if (distance >= 30 ) {
            Serial.println(distance);  //แสดงค่าระยะทาง
            Serial.print("sum : )");
            Serial.println(lowlevel);
            lowlevel += 1;
          } else {
            digitalWrite(solinoid, HIGH);
            digitalWrite(relay1, LOW);
            digitalWrite(relay2, LOW);
            digitalWrite(relay3, LOW);
            digitalWrite(relay4, LOW);
            Serial.println(distance);
          }
          time_stop = millis();
          if (lowlevel > 5) { // น้ำเต็ม
            Serial.println("Status : out break");
            digitalWrite(solinoid, LOW);
            check_interupt = true;
            break;
          }
        }
      }
      if (check_interupt == true) {
        value_callback = "manaul_ready";
        Serial.println(value_callback);
        esp8266con.write(value_callback.c_str());
        delay(1000);
        check_interupt = false;
      }
    }
    else if (mode_working == "online") {
      digitalWrite(system_status, HIGH);
      Serial.println("Board Online");
    }
    else if (mode_working == "offline") {
      digitalWrite(system_status, LOW);
      Serial.println("Board offline");
    }
  }
}


void interruptf() {
  if (check_reset == true) {
    Serial.println("interupltt now!!");
    mode_working = "interupt";
    check_interupt = true;
    return mode_working;
    check_reset = false;
  }
}

void stopf() {
  Serial.println("Stop now!!");
  boolean stop_boolean = false;
  Serial.println("------------------------------------------");
  lowlevel = 1;
  while (lowlevel <= 5 && lowlevel >= 1) {
    if (millis() - time_stop > 1000) {
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);
      duration = pulseIn(echoPin, HIGH);
      distance = duration / 58.2;
      ////////////////////////////
      if (distance >= 30 ) {
        Serial.println(distance);  //แสดงค่าระยะทาง
        Serial.print("sum : )");
        Serial.println(lowlevel);
        lowlevel += 1;
      } else {
        digitalWrite(solinoid, HIGH);
        digitalWrite(relay1, LOW);
        digitalWrite(relay2, LOW);
        digitalWrite(relay3, LOW);
        digitalWrite(relay4, LOW);
        Serial.println(distance);
      }
      time_stop = millis();
      if (lowlevel > 5) { // น้ำเต็ม
        Serial.println("Status : out break");
        digitalWrite(solinoid, LOW);
        stop_boolean = true;
        break;
      }
    }
  }
  if (stop_boolean == true) {
    digitalWrite(relay1, LOW);
    digitalWrite(relay2, LOW);
    digitalWrite(relay3, LOW);
    digitalWrite(relay4, LOW);
    digitalWrite(solinoid, LOW);
    Serial.println("stop_success");
    value_callback = "stop_success";
    if (millis() - time_2 > 5000) {
    Serial.println(value_callback);
    esp8266con.write(value_callback.c_str());
    }
//    Serial.println(value_callback);
//    esp8266con.write(value_callback.c_str());
//    delay(1000);
    lowlevel = 1;
    stop_boolean = false;
    return loop();
  }
}

void clean_water_in () {  //ล้างบ่อ
  Serial.println("Start clean_water");
  ////// ดูดเข้า
  Serial.println("--------------------clean_water_in----------------------");
  Serial.print("pump port : ");
  Serial.println(pump_port);
  Serial.println("------------------------------------------");
  boolean first_in = false;
  while (maxlevel <= 5 && maxlevel >= 1) {
    esp8266con.listen();
    ReciveSerial();
    if (maxlevel <= 6) {
      if (millis() - time_1 > INTERVAL_MESSAGE1) {
        /////////////////////////////
        digitalWrite(trigPin, LOW);
        delayMicroseconds(2);
        digitalWrite(trigPin, HIGH);
        delayMicroseconds(10);
        digitalWrite(trigPin, LOW);
        duration = pulseIn(echoPin, HIGH);
        distance = (duration / 58.20);
        ////////////////////////////
        if (mode_working  == "interupt" || mode_working == "stop" ) {
          return loop();
        }
        if (distance <= 11) {
          Serial.println(distance);
          Serial.print("sum : )");
          Serial.println(maxlevel);
          maxlevel += 1;
        } else {
          digitalWrite(pump_port, HIGH); // เปิดปั้มน้ำเข้า
          digitalWrite(solinoid, LOW); // โซลินอย
          Serial.println(distance);
        }
        time_1 = millis();
        if (maxlevel > 5) { // น้ำเต็ม
          Serial.println("Status : in break");
          digitalWrite(pump_port, LOW); // ปิดปั้มน้ำเข้า
          first_in = true;
          break;
        }
      }
    }
  }
  //  ส่งสเตตัส status_water_1st_in
  if (first_in == true) {
    Serial.println("st_in");
    value_callback = "st_in";
    Serial.println(value_callback);
    delay(2000);
    esp8266con.write(value_callback.c_str());
    digitalWrite(relay1, LOW);
    maxlevel = 1;
    first_in = false;
    //    delay(2000);
    clean_water_out();
  }
}
/////////////////////////////////////////////////////////////////////////// ดูดออก
void clean_water_out() {
  Serial.println("------------------------clean_water_out------------------");
  Serial.print("pump port : ");
  Serial.println(pump_port);
  Serial.println("------------------------------------------");
  boolean first_out = false;
  lowlevel = 1;
  while (lowlevel <= 5 && lowlevel >= 1) {
    esp8266con.listen();
    ReciveSerial();
    if (millis() - time_1 > INTERVAL_MESSAGE1) {
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);
      duration = pulseIn(echoPin, HIGH);
      distance = duration / 58.2;
      ////////////////////////////
      if (mode_working  == "interupt" || mode_working == "stop") {
        return loop();
      }
      if (distance >= 30 ) {
        Serial.println(distance);  //แสดงค่าระยะทาง
        Serial.print("sum : )");
        Serial.println(lowlevel);
        lowlevel += 1;
      } else {
        digitalWrite(solinoid, HIGH);
        digitalWrite(pump_port, LOW);
        Serial.println(distance);
      }
      time_1 = millis();
      if (lowlevel > 5) { // น้ำเต็ม
        Serial.println("Status : out break");
        digitalWrite(solinoid, LOW);
        first_out = true;
        break;
      }
    }
  }
  //  ส่งสเตตัส status_water_1st_out
  if (first_out == true) {
    Serial.println("1st_out");
    value_callback = "1st_out";
    Serial.println(value_callback);
    esp8266con.write(value_callback.c_str());

    lowlevel = 1;
    first_out = false;
    check_water_in();
  }
}

void check_water_in() {
  Serial.println("Start check_water");
  ////// ดูดเข้า
  Serial.println("----------------------check_water_in--------------------");
  Serial.print("pump port : ");
  Serial.println(pump_port);
  Serial.println("------------------------------------------");
  boolean second_in = false;
  maxlevel = 1;
  while (maxlevel <= 5 && maxlevel >= 1) {
    esp8266con.listen();
    ReciveSerial();
    if (maxlevel <= 6) {
      if (millis() - time_1 > INTERVAL_MESSAGE1) {
        digitalWrite(trigPin, LOW);
        delayMicroseconds(2);
        digitalWrite(trigPin, HIGH);
        delayMicroseconds(10);
        digitalWrite(trigPin, LOW);
        duration = pulseIn(echoPin, HIGH);
        distance = duration / 58.2;
        ////////////////////////////
        if (mode_working  == "interupt" || mode_working == "stop") {
          return loop();
        }
        if (distance <= 11) {
          Serial.println(distance);  //แสดงค่าระยะทาง
          Serial.print("sum : )");
          Serial.println(maxlevel);
          maxlevel += 1;
        } else {
          digitalWrite(pump_port, HIGH);
          digitalWrite(solinoid, LOW);
          Serial.println(distance);
        }
        time_1 = millis();
        if (maxlevel > 5) { // น้ำเต็ม
          Serial.println("Status : in break");
          digitalWrite(pump_port, LOW);
          second_in = true;
          break;
        }
      }
    }
  }
  //  ส่งสเตตัส status_water_2nd_in
  if (second_in == true) {
    Serial.println("2nd_in");
    value_callback = "2nd_in";
    Serial.println(value_callback);
    esp8266con.write(value_callback.c_str());

    digitalWrite(solinoid, LOW);
    maxlevel = 1;
    second_in = false;
  }

  if (millis() - time_2 > INTERVAL_MESSAGE2) {
    Serial.println("รอน้ำนิ่ง");
    Serial.println("---");
    time_2 = millis();
  }
  delay(6000);
  test_water();
}
////////////////////////////////////////////
void test_water() {
  boolean succes_value = false;
  float ph_value = 0;
  float do_value = 0;
  float tur_value = 0;
  float temp_value = 0;
  //////////////////////// pH /////////////////////////////////////////////////////
  int buf[10]; //buffer for read analog
  for (int i = 0; i < 10; i++) //Get 10 sample value from the sensor for smooth the value
  {
    buf[i] = analogRead(SensorPin);
    delay(10);
  }
  for (int i = 0; i < 9; i++) //sort the analog from small to large
  {
    for (int j = i + 1; j < 10; j++)
    {
      if (buf[i] > buf[j])
      {
        int temp = buf[i];
        buf[i] = buf[j];
        buf[j] = temp;
      }
    }
  }
  avgValue = 0;
  for (int i = 2; i < 8; i++) //take the average value of 6 center sample
    avgValue += buf[i];
  float phValue = (float)avgValue * 5.0 / 1024 / 6; //convert the analog into millivolt
  ph_value = 3.5 * phValue + Offset; //convert the millivolt into pH value
  ////////////////////////////////////Tur ความขุ่น//////////////////////////////////////////
  int sensorTur = A14;
  float volt;
  float ntu;
  volt = 0;
  for (int i = 0; i < 800; i++)
  {
    volt += ((float)analogRead(sensorTur) / 1023) * 5;
  }
  volt = volt / 800;
  volt = round_to_dp(volt, 2);
  if (volt < 2.5) {
    ntu = 3000;
  } else {
    ntu = -1120.4 * square(volt) + 5742.3 * volt - 4353.8;
  }
  tur_value = abs(ntu);
  ///////////////////////////////// Temp อุณหภูมิ
  sensors.requestTemperatures();
  Serial.println(sensors.getTempCByIndex(0));
  temp_value = sensors.getTempCByIndex(0);
  ///////////////////////////////// Do
  boolean check_time_do = true;
  while (check_time_do == true) {
    esp8266con.listen();
    ReciveSerial();
    if (millis() - time_2 > 90000) {
      Temperaturet = (uint8_t)temp_value;
      ADC_Raw = analogRead(DO_PIN);
      ADC_Voltage = uint32_t(VREF) * ADC_Raw / ADC_RES;

      Serial.print("Temperaturet:\t" + String(Temperaturet) + "\t");
      Serial.print("ADC RAW:\t" + String(ADC_Raw) + "\t");
      Serial.print("ADC Voltage:\t" + String(ADC_Voltage) + "\t");
      Serial.println("DO:\t" + String((readDO(ADC_Voltage, Temperaturet))) + "\t");
      do_value = float(abs((readDO(ADC_Voltage, Temperaturet)) / 1000));
      Serial.println("-------------------------------");
      check_time_do = false;
      time_2 = millis();
    }
  }
  /////////////////////////////////
  succes_value = true;
  value_callback = String(ph_value) + "," + String(do_value) + "," + String(tur_value) + "," + String(temp_value);
  if (succes_value == true) {
    if (millis() - time_2 > 5000) {
      Serial.println("---");
      time_2 = millis();
    }

    esp8266con.write(value_callback.c_str());
    succes_value = false;
    if (millis() - time_2 > 5000) {
      Serial.println("---");
      time_2 = millis();
    }
  }
  pump_out();
  Serial.println("------------------------");
}
////// ดูดออก
void pump_out() {
  Serial.println("-----------------------pump_out-------------------");
  Serial.print("pump port : ");
  Serial.println(pump_port);
  Serial.println("------------------------------------------");
  boolean second_out = false;
  while (lowlevel <= 5 && lowlevel >= 1) {
    esp8266con.listen();
    ReciveSerial();
    if (millis() - time_1 > INTERVAL_MESSAGE1) {
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);
      duration = pulseIn(echoPin, HIGH);
      distance = duration / 58.2;
      ////////////////////////////
      if (mode_working  == "interupt" || mode_working == "stop") {
        return loop();
      }
      if (distance >= 31 ) {
        Serial.println(distance);  //แสดงค่าระยะทาง
        Serial.print("sum : )");
        Serial.println(lowlevel);
        lowlevel += 1;
      } else {
        digitalWrite(solinoid, HIGH);
        digitalWrite(pump_port, LOW);
        Serial.println(distance);
      }
      time_1 = millis();
      if (lowlevel > 5) { // น้ำเต็ม
        Serial.println("Status : out break");
        digitalWrite(solinoid, LOW);
        second_out = true;
        break;
      }
    }
  }
  if (millis() - time_2 > 5000) {
    Serial.println("---");
    time_2 = millis();
  }
  //  ส่งสเตตัส status_water_1st_out
  if (second_out == true) {
    Serial.println("2nd_out");
    value_callback = "2nd_out";
    Serial.println(value_callback);
    esp8266con.write(value_callback.c_str());

    digitalWrite(pump_port, LOW);
    lowlevel = 1;
    second_out = false;
  }

}
//function tur sensor
float round_to_dp( float in_value, int decimal_place )
{
  float multiplier = powf( 10.0f, decimal_place );
  in_value = roundf( in_value * multiplier ) / multiplier;
  return in_value;
}
