// import library
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>
#include <string>

// ชื่อ wifi / รหัส wifi 
#define WIFI_STA_NAME "Neo3"
#define WIFI_STA_PASS "))))))))"

// ชื่อ server ของ mqtt broker ยังใช้เป็นตัวทดลอง
#define MQTT_SERVER "iot.cpe.ku.ac.th"
// กำหนด port ปกติแล้ว mqtt จะใช้ 1883
#define MQTT_PORT 1883

// ข้อมูลต่างๆ
#define MQTT_USERNAME "b6310500350"
#define MQTT_PASSWORD "witnapat.c@ku.th"
#define MQTT_NAME ""

// กำหนด pin
#define AVOIDANCE 19
#define LED_YELLOW 33
#define LED_GREEN 27
#define LDR 34

Servo myservo;

// กำหนดให้ใช้เป็น mqtt
WiFiClient client;
PubSubClient mqtt(client);

static int is_out = 0, out = 0, old_out = 0, in = 0, sit = 0;
bool reset_av = true, reset_servo = true;
char s[10]; // สำหรับเปลี่ยนค่า int -> string

// ฟังชันก์ callback ซึ่งจะทำเมื่อมีการ publish มาจาก server แล้วเรามีการ subscribe ไว้นั่นเอง
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println(topic);
  payload[length] = '\0';
  String topic_str = topic, payload_str = (char*)payload;
  Serial.println("[" + topic_str + "]: " + payload_str);

  // สั่งให้ทำงานตามข้อมูลที่ได้จากการ subscribe
  if (topic_str == "b6310500350/train/in") {
    myservo.write(0); // สั่งให้ Servo หมุนไปองศาที่ 0
    sit = 0; // reset ว่าคนในรถไฟว่าง
  }
  if (topic_str == "b6310500350/fast/in") {
    in = atoi((char*)payload); // รับค่าจำนวนคนที่เข้าคิว
  }
  if (topic_str == "b6310500350/train/sit") {
    sit = atoi((char*)payload); // รับค่าจำนวนคนที่เข้าคิว
  }
}

// ฟังชันก์ในการเชื่อมต่อ wifi
void connectToWiFi() {
  Serial.println("Connected...");
 
  // ทำการเชื่อมต่อ wifi
  WiFi.begin(WIFI_STA_NAME, WIFI_STA_PASS);
  
  // วนไปเรื่อยถ้ายังเชื่อมต่อไม่ได้
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  
  // เช็ค IP Address
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  // เรียกฟังชันก์เชื่อมต่อ wifi
  connectToWiFi();

  // เชื่อมต่อ mqtt server
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  myservo.attach(18);
  mqtt.setCallback(callback);
}

void loop() {
  // ถ้ายังไม่ได้เชื่อมต่อก็เข้า if นี้ ก็คือเริ่มการเชื่อมต่อนั่นเอง
  if (!mqtt.connected()) {
    Serial.print("MQTT connection... ");
    
    // วนไปเรื่อยๆ เมื่อยังเชื่อมต่อไม่ได้
    while (!mqtt.connected()) {
      Serial.println("Reconnecting to MQTT Broker..");

      // เช็คการเชื่อมต่อว่าสำเร็จหรือยัง พร้อมกำหนด public และ subscribe 
      if (mqtt.connect(MQTT_NAME, MQTT_USERNAME, MQTT_PASSWORD)) {
        Serial.println("Connected.... Success");
        mqtt.subscribe("b6310500350/train/in");
        mqtt.subscribe("b6310500350/fast/in");
        mqtt.subscribe("b6310500350/train/sit");
      }
    }
  } else {
    mqtt.loop();
  }

  is_out = !digitalRead(AVOIDANCE); //อ่านค่าสัญญาณ digital ขา19 ที่ต่อกับ avoidance ถ้า คนผ่านจะเป็น 1

  if(!is_out) // debounce avoidance ต้องรอคนออกจากเส้นก่อนถึงจะนับเพิ่มได้
    reset_av = true; 
    
  if (is_out && reset_av) { // ถ้ามีคนผ่าน
    reset_av = false; 
    reset_servo = true;
    out++; // เพิ่มจำนวนคนที่ออก
    Serial.println(out); 
    mqtt.publish("b6310500350/fast/out", itoa(out, s, 10)); // publish จำนวนคนที่ออก
    mqtt.publish("b6310500350/train/sit", itoa(out - old_out, s, 10));  // publish ว่าคนขึ้นรถไฟกี่คน
  }

  if (out-old_out == 3 || ((out-in == 0) && (sit > 0) && reset_servo)) { // ถ้าคนออก 3 คน หรือ ถ้า fast ไม่มีคนแล้วมีคนขึ้นรถไฟ จะส่งไปให้ normal 
    myservo.write(90); // สั่งให้ Servo หมุนไปองศาที่ 90
    mqtt.publish("b6310500350/servo/normal", "1"); // publish ให้แถวปกติเปิด servo
    Serial.println(9999999); // debug ว่าสั่งให้ servo normal เปิด
    old_out = out;
    reset_servo = false;
  }
   
  delay(500);
}