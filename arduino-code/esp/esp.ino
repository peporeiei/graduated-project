#include <SoftwareSerial.h>

#include <ESP8266WiFi.h>
//#include <SoftwareSerial.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
const char* ssid = "NICA-IOT";  //สร้างตัวแปรไว้เก็บชื่อ ssid ของ AP ของเรา
const char* pass = "P@ss12345678#";  //สร้างตัวแปรไว้เก็บชื่อ password ของ AP ของเรา
// Config MQTT Server /////////////
const char* mqttServer = "188.166.228.186";
const int mqttPort = 1883;
const char* mqttUser = "mymqtt";
const char* mqttPassword = "a13013531";
WiFiClient espClient;
PubSubClient client(espClient);
////////////////////millis();////////////////////////
#define INTERVAL_MESSAGE1 30000  //1นาที
unsigned long time_1 = 0;
#define INTERVAL_MESSAGE2 100  //1นาที
unsigned long time_2 = 0;
void print_time(unsigned long time_millis);
/**
   NodeMCU v3 (ESP8266)   TX = GPIO 4 (D2)

   RX = GPIO 5 (D1)
*/
String poor_name = "", poor_id = "", pump_port = "", mode_working = "";

SoftwareSerial arduinocon(5, 4);
void callback(char* topic, byte* payload, unsigned int length) {
  //  time_2 = millis();

  Serial.print("Message arrived in topic: ");
  Serial.println(topic);

  Serial.print("Message:");
  /////////////// วนรับค่าออกมาใช้
  payload[length] = '\0';
  String topic_str = topic, payload_str = (char*)payload;
  Serial.println("[" + topic_str + "]: " + payload_str);
  Serial.println(topic_str);
  Serial.println("-----------------------");
  if (topic_str == "project/poor_request") {
    if (payload_str != "") {
      Serial.println(String(payload_str));
      StaticJsonDocument<200> doc_payload;
      DeserializationError error = deserializeJson(doc_payload, payload_str);
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
      }
      delay(1000);
      const int poorID = doc_payload["poorID"];
      const char* poorName = doc_payload["poorName"];
      int pump_port = doc_payload["pump_port"];
      const char* modes = doc_payload["mode"];

      poor_name = String(poorName);
      poor_id = poorID;
      pump_port = pump_port;
      mode_working = modes;
      delay(1000);
      const size_t CAPACITY = JSON_OBJECT_SIZE(1);
      StaticJsonDocument<1024> doc;
      ///////////// create an object
      JsonObject object = doc.to<JsonObject>();
      object["pump_port"] = pump_port;
      object["mode"] = mode_working;
      String all;
      serializeJson(doc, all);
      Serial.println(all);

      if (mode_working == "interupt") {
        delay(1000);
        arduinocon.write(all.c_str());
        Serial.write(all.c_str());
        Serial.println("MODE : interupt");
        Serial.println("-----------------------------------------");

      } else if (mode_working == "auto") {
        delay(1000);
        arduinocon.write(all.c_str());
        Serial.write(all.c_str());

      } else if (mode_working == "manaul") {
        delay(1000);
        arduinocon.write(all.c_str());
        Serial.write(all.c_str());

      } else if (mode_working == "stop") {
        arduinocon.write(all.c_str());
        Serial.write(all.c_str());
        digitalWrite(0, HIGH);
        Serial.println("MODE : Stop");
        Serial.println("-----------------------------------------");
      }
    }
  }
  //  else if (topic_str == "project/mode") {
  //    if (payload_str != "" && payload_str == "manaul") {
  //
  //    }
  //  }
  digitalWrite(16, LOW);
  digitalWrite(0, LOW);
}

void setup() {
  Serial.begin(115200);
  arduinocon.begin(57600);
  pinMode(16, OUTPUT);
  pinMode(0, OUTPUT);
  pinMode(23, OUTPUT);
  digitalWrite(16, HIGH);
  digitalWrite(0, HIGH);
  digitalWrite(23, LOW);

  Serial.println("\n\n\n water level start");
  wificonnect();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    client.publish("project/mode", "offline");
    if (client.connect("ESP32Client", mqttUser, mqttPassword)) {

      Serial.println("connected");

    } else {

      Serial.print("failed with state ");
      Serial.print(client.state());
      const size_t CAPACITY = JSON_OBJECT_SIZE(1);
      StaticJsonDocument<1024> doc;
      ///////////// create an object
      JsonObject object = doc.to<JsonObject>();
      object["pump_port"] = 0;
      object["mode"] = "offline";
      String all;
      serializeJson(doc, all);
      Serial.println(all);
      arduinocon.write(all.c_str());
      Serial.write(all.c_str());
      delay(2000);
    }
  }
  client.subscribe("project/mode");
  client.subscribe("project/poor_response");
  client.subscribe("project/poor_request");
  client.subscribe("project/manaul/status");
  Serial.println("-----------------------");
  client.publish("project/mode", "online");
  const size_t CAPACITY = JSON_OBJECT_SIZE(1);
  StaticJsonDocument<1024> doc;
  ///////////// create an object
  JsonObject object = doc.to<JsonObject>();
  object["pump_port"] = 0;
  object["mode"] = "online";
  String all;
  serializeJson(doc, all);
  Serial.println(all);
  arduinocon.write(all.c_str());
  Serial.write(all.c_str());
  /////////////////////////////////////
}

void wificonnect() {
  WiFi.begin(ssid, pass);  //ทำการเชื่อมต่อไปยัง AP

  while (WiFi.status() != WL_CONNECTED) {  //รอจนกว่าจะเชื่อมต่อสำเร็จ
    Serial.print(".");                     //แสดง ... ไปเรื่อยๆ จนกว่าจะเชื่อมต่อได้
    delay(500);
  }  //ถ้าเชื่อมต่อสำเร็จก็จะหลุก loop while
  Serial.println("");
  Serial.println("Wi-Fi connected");  //แสดงว่าเชื่อมต่อ Wi-Fi ได้แล้ว
  Serial.print("IP Address : ");
  Serial.println(WiFi.localIP());  //แสดง IP ของบอร์ดที่ได้รับแจกจาก AP
}

void loop() {
  client.loop();
  digitalWrite(16, LOW);
  digitalWrite(0, LOW);
  if (!client.connected()) {
    client.publish("project/mode", "offline");
    while (!client.connected()) {
      Serial.println("Connecting to MQTT...");
      if (client.connect("ESP32Client", mqttUser, mqttPassword)) {
        Serial.println("connected");
      } else {
        Serial.print("failed with state ");
        Serial.print(client.state());
        delay(2000);
      }
    }
    client.subscribe("project/mode");
    client.subscribe("project/poor_response");
    client.subscribe("project/poor_request");
    client.subscribe("project/manaul/status");
    client.publish("project/mode", "online");
  }
  /**
     เช็คว่าถ้ามีค่าเข้ามาจาก Serial monitor ให้วนรับค่า
     และแสดงออกทาง arduino ทีละตัวอักษร
     และแสดงออกทาง Serial monitor ทีละตัวอักษร จนครบทุกตัวอักษร
     และออกจาก loop
  */
  while (Serial.available() > 0) {
    char inByte = Serial.read();
    arduinocon.write(inByte);
    Serial.write(inByte);
  }
  /**
     เช็คว่าถ้ามีค่าเข้ามาจาก arduino ให้วนรับค่า
     และแสดงออกทาง Serial monitor ทีละตัวอักษร จนครบทุกตัวอักษร
     และออกจาก loop
  */
  String value_serial = "";
  boolean poor1 = false;
  while (arduinocon.available() > 0) {
    if (millis() - time_2 > 50) {
      //    arduinocon.listen();
      char inByte = arduinocon.read();
      value_serial += String(inByte);
      Serial.print("value_serial : ");
      Serial.println(value_serial);
      //      Serial.write(inByte);
      //      switch (inByte)
      //      {
      //        case 'a':
      //          Serial.print("case a : ");
      //          Serial.println(value_serial);
      //          poor1 = true;
      //          break;
      //      }
      time_2 = millis();
    }
  }
  delay(1000);
  if (value_serial != "") {
    Serial.println("----------------------------");
    Serial.println(value_serial);
    //สถานะน้ำเต็ม1
    if (value_serial == "st_in") {
      Serial.print("Publish message: ");
      Serial.println(value_serial);
      Serial.println("-----xxx------");
      const size_t CAPACITY = JSON_OBJECT_SIZE(1);
      StaticJsonDocument<1024> doc;
      ///////////// create an object
      JsonObject object = doc.to<JsonObject>();
      object["poorID"] = poor_id;
      object["poorName"] = poor_name;
      object["status"] = "status_water_1st_in";
      String all;
      serializeJson(doc, all);
      Serial.println(all);
      Serial.println("-----------------");
      //    //------------------------------------------
      client.publish("project/poor_response", all.c_str());
      //    //------------------------------------------
      value_serial = "";
      //สถานะน้ำออก2
    } else if (value_serial == "1st_out") {
      Serial.print("Publish message: ");
      Serial.println(value_serial);
      Serial.println("-----xxx------");
      const size_t CAPACITY = JSON_OBJECT_SIZE(1);
      StaticJsonDocument<1024> doc;
      ///////////// create an object
      JsonObject object = doc.to<JsonObject>();
      object["poorID"] = poor_id;
      object["poorName"] = poor_name;
      object["status"] = "status_water_1st_out";
      String all;
      serializeJson(doc, all);
      Serial.println(all);
      Serial.println("-----------------");
      //    //------------------------------------------
      client.publish("project/poor_response", all.c_str());
      //    //------------------------------------------
      value_serial = "";

    } else if (value_serial == "2nd_in") {
      Serial.print("Publish message: ");
      Serial.println(value_serial);
      Serial.println("-----xxx------");
      const size_t CAPACITY = JSON_OBJECT_SIZE(1);
      StaticJsonDocument<1024> doc;
      ///////////// create an object
      JsonObject object = doc.to<JsonObject>();
      object["poorID"] = poor_id;
      object["poorName"] = poor_name;
      object["status"] = "status_water_2nd_in";
      String all;
      serializeJson(doc, all);
      Serial.println(all);
      Serial.println("-----------------");
      //    //------------------------------------------
      client.publish("project/poor_response", all.c_str());
      //    //------------------------------------------
      value_serial = "";

    } else if (value_serial == "2nd_out") {
      Serial.print("Publish message: ");
      Serial.println(value_serial);
      Serial.println("-----xxx------");
      const size_t CAPACITY = JSON_OBJECT_SIZE(1);
      StaticJsonDocument<1024> doc;
      ///////////// create an object
      JsonObject object = doc.to<JsonObject>();
      object["poorID"] = poor_id;
      object["poorName"] = poor_name;
      object["status"] = "status_water_2nd_out";
      String all;
      serializeJson(doc, all);
      Serial.println(all);
      Serial.println("-----------------");
      //    //------------------------------------------
      client.publish("project/poor_response", all.c_str());
      //    //------------------------------------------
      value_serial = "";

    } else if (value_serial == "manaul_ready") {
      Serial.println("-----------------------------------------");
      Serial.println("Mega Call : manaul_ready");
      client.publish("project/manaul/status", "manaul_ready");
      Serial.println("-----------------------------------------");
    } else if (value_serial == "stop_success") {
      Serial.println("-----------------------------------------");
      Serial.println("Mega Call : stop_success");
      client.publish("project/manaul/status", "stop_success");
      Serial.println("-----------------------------------------");
    } else {
      Serial.print("Publish message: ");
      Serial.println(value_serial);
      Serial.println("-----xxx------");
      const size_t CAPACITY = JSON_OBJECT_SIZE(1);
      StaticJsonDocument<1024> doc;
      ///////////// create an object
      JsonObject object = doc.to<JsonObject>();
      object["poorID"] = poor_id;
      object["value_sensor"] = String(value_serial);
      String all;
      serializeJson(doc, all);
      Serial.println(all);
      Serial.println("-----------------");
      //    //------------------------------------------
      client.publish("project/sensor_value", all.c_str());
      //    //------------------------------------------
      value_serial = "";
    }
  }
}
