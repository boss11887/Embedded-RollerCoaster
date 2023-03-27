#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <ESP32Servo.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define WIFI_SSID <YOUR WIFI NAME>
#define WIFI_PASS <YOUR WIFI PASSWORD>
#define MQTT_BROKER <YOUR MQTT SERVER>
#define MQTT_USER <YOUR MQTT USERNAME>
#define MQTT_PASS <YOUR MQTT PASSWORD>
#define TOPIC_PREFIX "b6310500350"

#define TRAIN_MAX 5
#define MAX_NORMAL 2
#define MAX_FAST 3
#define SERVO_PORT 23
// open 90 close 30
#define LDR_PORT 36
#define TOPIC_TRAIN_SIT TOPIC_PREFIX "/train/sit"
#define TOPIC_NORMAL_IN TOPIC_PREFIX "/normal/in"
#define TOPIC_NORMAL_OUT TOPIC_PREFIX "/normal/out"
#define TOPIC_FAST_OUT TOPIC_PREFIX "/fast/out"
#define TOPIC_SERVO_NORMAL TOPIC_PREFIX "/servo/normal"
#define OLED_RESET 16

WiFiClient wifiClient;
PubSubClient mqtt(MQTT_BROKER, 1883, wifiClient);
uint32_t last_publish;
Servo servo_normal;
Adafruit_SSD1306 display(OLED_RESET);
// static int state = 0;   // 0 off 1 on
static int person_in = 0;
static int person_out = 0;
static int train_sit = 0;
static int nub = 0;

enum servo_state
{
  OPEN,
  CLOSE
};
enum ldr_state
{
  DETECTING_PEOPLE,
  DEBOUNCE1,
  UNDETECT_PEOPLE,
  DEBOUNCE2
};
servo_state state;
ldr_state nub_state;
uint32_t timestamp1;

void connect_wifi()
{
  printf("WiFi MAC address is %s\n", WiFi.macAddress().c_str());
  printf("Connecting to WiFi %s.\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    printf(".");
    fflush(stdout);
    delay(500);
  }
  printf("\nWiFi connected.\n");
}

void OLED_print_open()
{
  char s[10];
  display.clearDisplay();      // ลบภาพในหน้าจอทั้งหมด
  display.setTextSize(2);      // กำหนดขนาดตัวอักษร
  display.setTextColor(WHITE); // กำหนดข้อความสีขาว
  display.setCursor(60, 10); // กำหนดตำแหน่ง x,y ที่จะแสดงผล
  display.println(itoa(train_sit - nub, s, 10));
  display.display();
}

void OLED_print_close()
{
  char s[10];
  display.clearDisplay();      // ลบภาพในหน้าจอทั้งหมด
  display.setTextSize(2);      // กำหนดขนาดตัวอักษร
  display.setTextColor(WHITE); // กำหนดข้อความสีขาว
  display.setCursor(10, 10);   // กำหนดตำแหน่ง x,y ที่จะแสดงผล
  display.println("CLOSED");
  display.display();
}

void mqtt_callback(char *topic, byte *payload, unsigned int length)
{
  if (strcmp(topic, TOPIC_SERVO_NORMAL) == 0)
  {
    payload[length] = 0; 
    int value = atoi((char *)payload);
    if(value == 1){
      state = OPEN;
      nub = 0;
      // printf("Door Open\n");
      // printf("pass = %d ... train sit = %d ... person_in = %d ... person_out = %d \n", nub, train_sit, person_in, person_out);
      servo_normal.write(90); 
      delay(100);
    }
    else if(value == 0){
      state = CLOSE;
      nub = 0;
      // printf("Door Close\n");
      servo_normal.write(30); 
      delay(100);
    }
    else{}
  }

  if (strcmp(topic, TOPIC_NORMAL_IN) == 0)
  {
    payload[length] = 0;
    person_in = atoi((char *)payload);
    printf("get person_in = %d\n", person_in);
  }

  if (strcmp(topic, TOPIC_TRAIN_SIT) == 0)
  {
    if(state==0){
      payload[length] = 0;
      train_sit = TRAIN_MAX - atoi((char *)payload);
      printf("get train_sit = %d\n", train_sit);
    }
  }
}

void connect_mqtt()
{
  printf("Connecting to MQTT broker at %s.\n", MQTT_BROKER);
  if (!mqtt.connect("", MQTT_USER, MQTT_PASS))
  {
    printf("Failed to connect to MQTT broker.\n");
    for (;;)
    {
    } // wait here forever
  }
  mqtt.setCallback(mqtt_callback);
  mqtt.subscribe(TOPIC_SERVO_NORMAL);
  mqtt.subscribe(TOPIC_TRAIN_SIT);
  mqtt.subscribe(TOPIC_NORMAL_IN);
  printf("MQTT broker connected.\n");
}

void setup()
{
  Serial.begin(115200);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c); // สั่งให้จอ OLED เริ่มทำงานที่ Address 0x3C
  servo_normal.attach(SERVO_PORT);
  pinMode(LDR_PORT, INPUT);
  connect_wifi();
  person_out = 0;
  person_in = 0;
  train_sit = MAX_NORMAL;
  timestamp1 = millis();
  state = CLOSE;
  connect_mqtt();
  servo_normal.write(30);
  mqtt.publish(TOPIC_NORMAL_OUT, "0");
  mqtt.publish(TOPIC_FAST_OUT, "0");
  OLED_print_close();
  // printf("pass = %d ... train sit = %d ... person_in = %d ... person_out = %d \n", nub, train_sit, person_in, person_out);
}

void task1()
{
  uint32_t now;
  if (nub_state == DETECTING_PEOPLE)
  {
    if (analogRead(LDR_PORT) < 1000)
    { // meet people
      nub_state = DEBOUNCE1;
      nub++;
      char sss[10];
      mqtt.publish("b6310500350/normal/out", itoa(person_out + nub, sss, 10));
      // mqtt.publish("b6310500350/normal/in", itoa(normal_in, ss, 10));
      // printf("Current Normal is %d \n", normal_in);
      timestamp1 = millis();
    }
  }
  else if (nub_state == DEBOUNCE1)
  {
    now = millis();
    if (now - timestamp1 >= 111)
    {
      nub_state = UNDETECT_PEOPLE;
      timestamp1 = now;
    }
  }
  else if (nub_state == UNDETECT_PEOPLE)
  {
    if (analogRead(LDR_PORT) > 1000)
    { // Detect people
      nub_state = DEBOUNCE2;
      timestamp1 = millis();
    }
  }
  else if (nub_state == DEBOUNCE2)
  {
    now = millis();
    if (now - timestamp1 >= 111)
    {
      nub_state = DETECTING_PEOPLE;
      timestamp1 = now;
    }
  }
}

void loop()
{
  if (!mqtt.connected()){
    connect_mqtt();
  }
  else{
    mqtt.loop();
    if(state==OPEN){
      task1();
      OLED_print_open();
      if(person_out+nub == person_in){
        // printf("%d %d %d\n", person_out, person_in, nub);
        char ss[10];
        mqtt.publish("b6310500350/normal/out", itoa(person_out + nub, ss, 10));
        mqtt.publish("b6310500350/servo/normal", "0");
        person_out += nub;
        state = CLOSE;
      }
      else if(train_sit-nub==0){
        // printf("%d %d %d\n", person_out, person_in, nub);
        char ss[10];
        mqtt.publish("b6310500350/normal/out", itoa(person_out + nub, ss, 10));
        mqtt.publish("b6310500350/servo/normal", "0");
        person_out += nub;
        state = CLOSE;
      }
    }
    else{
      OLED_print_close();
    }
    
    }

  // uint32_t save = millis();
  // if (save - last_publish >= 2000)
  // {
  //   printf("%d %d\n", nub, analogRead(LDR_PORT));
  //   last_publish = save;
    
  // }
}
