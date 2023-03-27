// import library
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <string>

// ชื่อ wifi / รหัส wifi 
#define WIFI_STA_NAME <YOUR WIFI NAME>
#define WIFI_STA_PASS <YOUR WIFI PASSWORD>

// ชื่อ server ของ mqtt broker 
#define MQTT_SERVER <YOUR MQTT SERVER>
// กำหนด port ปกติแล้ว mqtt จะใช้ 1883
#define MQTT_PORT 1883

// ข้อมูลต่างๆ
#define MQTT_USERNAME <YOUR MQTT USERNAME>
#define MQTT_PASSWORD <YOUR MQTT PASSWORD>
#define MQTT_NAME <YOUR MQTT NAME>

// กำหนด pin
#define LDR 36
#define OLED_RESET 16
Adafruit_SSD1306 display(OLED_RESET);
Servo myservo;

// กำหนดให้ใช้เป็น mqtt
WiFiClient client;
PubSubClient mqtt(client);

// ตัวแปร
static int is_out = 0, out = 0, old_out = 0, in = 0, sit = 0; // สำหรับนับคนเข้า/ออก คนในรถไฟ
bool reset_ldr = true, servo_on = false; // สำหรับ debounce 
char s[10]; // สำหรับเปลี่ยนค่า int -> string

// ฟังก์ชันสำหรับแสดงผลจอ OLED
void OLED_print(){
  display.clearDisplay(); // ลบภาพในหน้าจอทั้งหมด
  display.setTextSize(2); // กำหนดขนาดตัวอักษร
  display.setTextColor(WHITE); //กำหนดข้อความสีขาว 

  int remain = 3 - (out - old_out);
  if(remain == 0 || (out == 0 && !servo_on)){
    display.setCursor(30,10); // กำหนดตำแหน่ง x,y ที่จะแสดงผล
    display.println("CLOSED");
  }
    
  else{
    display.setCursor(60,10); // กำหนดตำแหน่ง x,y ที่จะแสดงผล
    display.println(itoa(3 - (out - old_out), s, 10));
  }
    
  display.display();
}

// ฟังก์ชัน callback ซึ่งจะทำเมื่อมีการ publish มาจาก server แล้วเรามีการ subscribe ไว้
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println(topic);
  payload[length] = '\0';
  String topic_str = topic, payload_str = (char*)payload;
  Serial.println("[" + topic_str + "]: " + payload_str);

  // สั่งให้ทำงานตามข้อมูลที่ได้จากการ subscribe
  if (topic_str == "b6310500350/train/in") {
    myservo.write(10); // สั่งให้ Servo หมุนไปองศาที่ 10
    sit = 0; // reset ว่าคนในรถไฟว่าง
    servo_on = true; // เก็บค่าว่า servo เปิด
    delay(1000);

    OLED_print();
  }
  if (topic_str == "b6310500350/fast/in") {
    in = atoi((char*)payload); // รับค่าจำนวนคนที่เข้าคิว
  }
  if (topic_str == "b6310500350/train/sit") {
    sit = atoi((char*)payload); // รับค่าจำนวนคนที่นั่งในรถไฟ
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
  myservo.attach(23);
  myservo.write(90); // เริ่มต้นให้ Servo หมุนไปองศาที่ 90

  out = 0, old_out = 0;
  mqtt.setCallback(callback);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3c); //สั่งให้จอ OLED เริ่มทำงานที่ Address 0x3C
  OLED_print();
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

  int read_ldr = analogRead(LDR); //อ่านค่าสัญญาณ LDR ถ้า คนผ่านจะน้อยกว่า 200
  
  if(read_ldr < 200)
    is_out = 1; // ถ้าคนผ่านจะเป็น 1
  else is_out = 0; // ถ้าคนออกจะกลับเป็น 0

  if(!is_out) // debounce ldr ต้องรอคนออกจากเส้นก่อนถึงจะนับเพิ่มได้
    reset_ldr = true; 
    
  if (is_out && reset_ldr && servo_on) { // ถ้ามีคนผ่าน
    reset_ldr = false; 
    out++; // เพิ่มจำนวนคนที่ออก
    Serial.println(out); 
    mqtt.publish("b6310500350/fast/out", itoa(out, s, 10)); // publish จำนวนคนที่ออก
    mqtt.publish("b6310500350/train/sit", itoa(out - old_out, s, 10));  // publish ว่าคนขึ้นรถไฟกี่คน

    OLED_print();
  }

  if (out-old_out == 3 || ((out-in == 0) && (sit > 0) && servo_on)) { // ถ้าคนออก 3 คน หรือ ถ้า fast ไม่มีคนแล้วมีคนขึ้นรถไฟ จะส่งไปให้ normal 
    myservo.write(90); // สั่งให้ Servo หมุนไปองศาที่ 90
    mqtt.publish("b6310500350/servo/normal", "1"); // publish ให้แถวปกติเปิด servo
    servo_on = false;
    OLED_print();
    old_out = out;
  }
   
  delay(500);
}
