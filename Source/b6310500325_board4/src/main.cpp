#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <string>
#include <iostream>
#include <Servo.h>
using namespace std;
Servo servoN;
Servo servoF;

#define WIFI_SSID <YOUR WIFI NAME>
#define WIFI_PASS <YOUR WIFI PASSWORD>
#define MQTT_BROKER <YOUR MQTT SERVER>
#define MQTT_USER <YOUR MQTT USERNAME>
#define MQTT_PASS <YOUR MQTT PASSWORD>
#define TOPIC_PREFIX ""

#define TOPIC_LIGHT TOPIC_PREFIX "/light"
#define TOPIC_LED_RED TOPIC_PREFIX "/led/red"

//Setport
#define LdrN 36
#define LdrF 39
#define motorN 19
#define motorF 21
#define metalN 9
#define metalF 10
#define buzzer 27

WiFiClient wifiClient;
PubSubClient mqtt(MQTT_BROKER, 1883, wifiClient);

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

void mqtt_callback(char *topic, byte *payload, unsigned int length)
{
  if (strcmp(topic, TOPIC_LED_RED) == 0)
  {
    payload[length] = 0; // null-terminate the payload to treat it as a string
    int value = atoi((char *)payload);
    printf("TOPIC_LED_RED.\n");
  }
}

void connect_mqtt()
{
  printf("Connecting to MQTT broker at %s.\n", MQTT_BROKER);
  if (!mqtt.connect("", MQTT_USER, MQTT_PASS))
  {
    printf("Failed to connect to MQTT broker.\n");
  }
  mqtt.setCallback(mqtt_callback);
  mqtt.subscribe(TOPIC_LED_RED);
  printf("MQTT broker connected.\n");
}

// สร้างของ fast pass
enum task1_state_t
{
  DETECTING_PEOPLE,
  WARN,
  DEBOUNCE1,
  UNDETECT_PEOPLE,
  DEBOUNCE2,
};
enum task2_state_t
{
  F_DETECTING_PEOPLE,
  F_WARN,
  F_DEBOUNCE1,
  F_UNDETECT_PEOPLE,
  F_DEBOUNCE2,
};
task1_state_t task1_state;
task2_state_t task2_state;
uint32_t timestamp1, timestamp2;
static int normal_in = 0;
static int fast_in = 0;

//Normal queue
void task1()
{
  uint32_t now;
  //printf("%d\n",analogRead(LdrN));
  if (task1_state == DETECTING_PEOPLE)
  {
    if(!digitalRead(metalN)){
      task1_state = WARN;
      timestamp1 = millis();
      digitalWrite(buzzer,1);
    }
    if (analogRead(LdrN) <= 500)
    { // meet people
      //printf("====%d\n",analogRead(LdrN));
      task1_state = DEBOUNCE1;
      normal_in++;
      char ss[10];
      mqtt.publish("b6310500350/normal/in", itoa(normal_in, ss, 10));
      delay(50);
      printf("Current Normal is %d \n", normal_in);
      timestamp1 = millis();
      servoN.write(90);
      delay(50);
    }
  }
  else if(task1_state == WARN){
    now = millis();
    if (now - timestamp1 >= 3000)
    {
      task1_state = DETECTING_PEOPLE;
      timestamp1 = now;
      digitalWrite(buzzer,0);
    }
  }
  else if (task1_state == DEBOUNCE1)
  {
    now = millis();
    if (now - timestamp1 >= 300)
    {
      task1_state = UNDETECT_PEOPLE;
      timestamp1 = now;
    }
  }
  else if (task1_state == UNDETECT_PEOPLE)
  {
    if (analogRead(LdrN) > 1100)
    { // Detect people
      task1_state = DEBOUNCE2;
      timestamp1 = millis();
    }
  }
  else if (task1_state == DEBOUNCE2)
  {
    now = millis();
    if (now - timestamp1 >= 3000)
    {
      servoN.write(0);
      delay(50);
      task1_state = DETECTING_PEOPLE;
      timestamp1 = now;
    }
  }
}

void task2(){
  uint32_t now;
  //printf("%d\n",analogRead(LdrF));
  if (task2_state == F_DETECTING_PEOPLE)
  {
    if(!digitalRead(metalF)){
      task2_state = F_WARN;
      timestamp2 = millis();
      digitalWrite(buzzer,1);
    }
    if (analogRead(LdrF) <= 15)
    { // meet people
      task2_state = F_DEBOUNCE1;
      fast_in++;
      char ss[10];
      mqtt.publish("b6310500350/fast/in", itoa(fast_in, ss, 10));
      delay(50);
      //printf("Current Fast is %d \n", fast_in);
      timestamp2 = millis();
      servoF.write(0);
      delay(50);
    }
  }
  else if(task2_state == F_WARN){
    now = millis();
    if (now - timestamp2 >= 3000)
    {
      task2_state = F_DETECTING_PEOPLE;
      timestamp2 = now;
      digitalWrite(buzzer,0);
    }
  }
  else if (task2_state == F_DEBOUNCE1)
  {
    now = millis();
    if (now - timestamp2 >= 130)
    {
      task2_state = F_UNDETECT_PEOPLE;
      timestamp2 = now;
    }
  }
  else if (task2_state == F_UNDETECT_PEOPLE)
  {
    if (analogRead(LdrF) > 100)
    { //Detect people
      task2_state = F_DEBOUNCE2;
      timestamp2 = millis();
    }
  }
  else if (task2_state == F_DEBOUNCE2)
  {
    now = millis();
    if (now - timestamp2 >= 3000)
    {
      task2_state = F_DETECTING_PEOPLE;
      timestamp2 = now;
      servoF.write(90);
      delay(50);
    }
  }
}

void setup()
{
  Serial.begin(115200);

  connect_wifi();
  connect_mqtt();
  mqtt.publish("b6310500350/normal/in", "0");
  mqtt.publish("b6310500350/fast/in", "0");
  //DeTectPeople
  pinMode(LdrN, INPUT);
  pinMode(LdrF, INPUT);
  pinMode(motorN, INPUT);
  pinMode(motorF, INPUT);
  pinMode(metalN, INPUT);
  pinMode(metalF, INPUT);
  pinMode(buzzer,OUTPUT);
  servoN.attach(motorN);
  servoN.write(0);
  servoF.attach(motorF);
  servoF.write(90);
  
  task1_state = DETECTING_PEOPLE;
  task2_state = F_DETECTING_PEOPLE;
  timestamp1 = millis();
  timestamp2 = millis();

}

void loop()
{
  if (!mqtt.connected())
  {
    connect_mqtt();
  }
    task1();
    task2();
}
