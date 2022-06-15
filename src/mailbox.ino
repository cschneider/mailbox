#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "WiFiManager.h"
#include <MQTTClient.h>
#include <SPI.h>
#include <Servo.h> 
 
const char* host = "mailbox";
const char* broker = "192.168.0.126";
const char* mqttUser = "mqttuser";
const char* mqttPass = "mqttpassword";

const int m_time = 5000;      //Meassuretime in Seconds

const int PIN_LID = D5;
const int PIN_DOOR = D6;
const int M_OPEN = 1;
const int M_CLOSED = 0;

const int SERVO_UP = 0;
const int SERVO_DOWN = 180;

Servo myservo;
WiFiClient wifi;
MQTTClient mqtt;
WiFiManager wifiManager;

int prevLid;
int prevDoor;
unsigned long oldMillis;


void connect();


void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
}


void setup(void){
  wifiManager.setAPCallback(configModeCallback);
  if(!wifiManager.autoConnect()) {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  } 
  myservo.attach(D1); 
  Serial.begin(115200);
  Serial.println();
 
  mqtt.begin(broker, wifi);
  mqtt.publish("/mailbox/resetReason", ESP.getResetReason());
 
  connect();
  Serial.println("Up and running!");
  myservo.write(SERVO_DOWN);
  prevLid = M_CLOSED;
  prevDoor = M_CLOSED;
}
 
void loop(void){
  interrupts();
  if(!mqtt.connected()) {
    connect();
  }
 
  mqtt.loop();
  pinMode(PIN_LID, INPUT_PULLUP);
  pinMode(PIN_DOOR, INPUT_PULLUP); 

  int lid = digitalRead(PIN_LID);
  int door = digitalRead(PIN_DOOR);

  if (prevLid != lid) {
    String lidState = lid == 1 ? "open" : "closed";
    mqtt.publish("/mailbox/lid", lidState);
    Serial.println("Lid " + lidState);
    if (lid == M_OPEN) {
      myservo.write(SERVO_UP);
    }
  }
  if (prevDoor != door) {
    String doorState = door == 1 ? "open" : "closed";
    mqtt.publish("/mailbox/door", doorState);
    Serial.println("door " + doorState);
    if (door == M_OPEN) {
      myservo.write(SERVO_DOWN);
    }
  }
  prevLid = lid;
  prevDoor = door;
  delay(100);
}
 
void connect() {
  Serial.print("WiFi connected. IP address: ");
  Serial.println(WiFi.localIP());
  blinkLED();
  Serial.print("Connecting to mqtt");
  while (!mqtt.connect(host, mqttUser, mqttPass)) {
    Serial.print(".");
  }
  Serial.println("\nconnected!");
  blinkLED();
}
 
void blinkLED() {
  pinMode(BUILTIN_LED, OUTPUT); // LED
  digitalWrite(BUILTIN_LED, LOW);
  delay(1000);
  digitalWrite(BUILTIN_LED, HIGH);
  delay(500);
}
