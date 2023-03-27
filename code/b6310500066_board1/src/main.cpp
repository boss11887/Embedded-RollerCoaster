// import library
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// define ตัวแปรที่ใช้
#define TRAIN_IN 19
#define TRAIN_OUT 18
#define LED 27

// ชื่อ wifi / รหัส wifi 
#define WIFI_STA_NAME <YOUR WIFI NAME>
#define WIFI_STA_PASS <YOUR WIFI PASSWORD>

// ชื่อ server ของ mqtt broker ยังใช้เป็นตัวทดลอง
#define MQTT_SERVER <YOUR MQTT SERVER>
// กำหนด port ปกติแล้ว mqtt จะใช้ 1883
#define MQTT_PORT 1883

// ข้อมูลต่างๆ
#define MQTT_USERNAME <YOUR MQTT USERNAME>
#define MQTT_PASSWORD <YOUR MQTT PASSWORD>
#define MQTT_NAME <YOUR MQTT NAME>

// กำหนดให้ใช้เป็น mqtt
WiFiClient client;
PubSubClient mqtt(client);

// กำหนดตัวแปรไว้เก็บ status การออกของรถไฟ และการมาของรถไฟ
static int statusGo = 1;
static int statusCome = 1;
static int running = 0;

// ฟังชันก์ callback ซึ่งจะทำเมื่อมีการ publish มาจาก server แล้วเรามีการ subscribe ไว้นั่นเอง
void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  String topic_str = topic, payload_str = (char*)payload;
  Serial.println("[" + topic_str + "]: " + payload_str);
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
  // กำหนด pin ของ led ให้เป็น input, output
  pinMode(TRAIN_IN, INPUT);
  pinMode(TRAIN_OUT, INPUT);
  pinMode(LED, OUTPUT);

  // เชื่อมต่อ mqtt server
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(callback);
}

void loop() {
  // put your main code here, to run repeatedly:
  // ถ้ายังไม่ได้เชื่อมต่อก็เข้า if นี้ ก็คือเริ่มการเชื่อมต่อนั่นเอง
  if (!mqtt.connected()) {
    Serial.print("MQTT connection... ");
    
    // วนไปเรื่อยๆ เมื่อยังเชื่อมต่อไม่ได้
    while (!mqtt.connected()) {
      Serial.println("Reconnecting to MQTT Broker..");
      
      // เช็คการเชื่อมต่อว่าสำเร็จหรือยัง พร้อมกำหนด public และ subscribe 
      if (mqtt.connect(MQTT_NAME,MQTT_USERNAME,MQTT_PASSWORD)) {
        Serial.println("Connected.... Success");
        // mqtt.publish("b6301500350/start","Hello World");
        mqtt.subscribe("b6310500350/train/in");
        mqtt.subscribe("b6310500350/train/out");
        // mqtt.subscribe("b6310500350/normal/in");
        // mqtt.subscribe("b6310500350/fast/in");
        // mqtt.subscribe("b6310500350/normal/out");
        // mqtt.subscribe("b6310500350/fast/out");
        mqtt.subscribe("b6310500350/servo/fast");
        // mqtt.subscribe("b6310500350/servo/normal");
        // mqtt.subscribe("b6310500350/train/sit");
      }
    }
  } else {
    // สั่งให้ mqtt ทำงานต่อไปเรื่อยๆ
    mqtt.loop();
  }

  // แสดงค่า avoidance
  Serial.print("TRAIN_IN = ");
  Serial.println(digitalRead(TRAIN_IN));
  Serial.print("TRAIN_OUT = ");
  Serial.println(digitalRead(TRAIN_OUT));

  // เช็คว่าถ้ารถไฟมาและรถไฟกำลังวิ่งอยู่ให้ดำเนินการ
  if (digitalRead(TRAIN_IN) == 0 && running == 1) {
    statusCome = 0;
    // วนไปเรื่อยๆ ถ้ายังรถไฟยังผ่านไม่หมดคันรถ
    while(statusCome == 0) {
      // ระหว่างนั้นก็ให้ LED ติดเพื่อบอกว่ารถไฟมาแล้ว
      digitalWrite(LED,1);
      // ถ้ารถไฟผ่านหมดคันรถก็ออกจากลูป
      if (digitalRead(TRAIN_IN) == 1) {
        statusCome = 1;
      }
      delay(100);
    }
    // บอกว่ารถไฟหยุดวิ่ง
    running = 0;
    // ทำการ publish บอกว่ารถไฟมาแล้ว
    // ปิด LED
    digitalWrite(LED,0);

    mqtt.publish("b6310500350/train/in","come");
    // ทำการ publish สั่งเปิด servo fast 
    // mqtt.publish("b6310500350/servo/fast",1);
  }
  
  // เช็คว่าถ้ารถไฟกำลังออกและรถไฟยังไม่วิ่งอยู่ให้ดำเนินการ
  if (digitalRead(TRAIN_OUT) == 0 && running == 0) {
    statusGo = 0;
    // วนไปเรื่อยๆ ถ้ายังรถไฟยังผ่านไม่หมดคันรถ
    while(statusGo == 0) {
      // ถ้ารถไฟผ่านหมดคันรถก็ออกจากลูป
      if (digitalRead(TRAIN_OUT) == 1) {
        statusGo = 1;
      }
      delay(100);
    }
    // บอกว่ารถไฟกำลังวิ่งอยู่
    running = 1;
    // ทำการ publish บอกว่ารถไฟไปแล้ว
    mqtt.publish("b6310500350/train/out","go");
  }
  delay(500);
}