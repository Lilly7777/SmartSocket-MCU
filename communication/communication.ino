#include <ESPFlash.h>
#include <ESPFlashCounter.h>
#include <ESPFlashString.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <WiFiClient.h>

#include "WebPages.h"

#include <vector>

#define LED_STATE_RED       D5
#define LED_STATE_GREEN     D6

#define RELAY_COM           4

#define RESET_BUTTON        D7

#define MQTT_SERVER_ADDRESS ""
#define MQTT_PORT 0
#define MQTT_USER ""
#define MQTT_PASSWORD ""

#define DEVICE_UID ""

typedef struct network_t{
    String ssid;
    String password;
    network_t(){}
    network_t(String ssid_, String password_){
      ssid = ssid_;
      password = password_; 
    }
} Network;

std::vector<Network> known_networks;

Network network;

ESP8266WebServer server(80);

WiFiClient wifi_client;
PubSubClient client(wifi_client);

ESPFlash<int> relay_status_eeprom("/relaystatus");
ESPFlashString ssid_eeprom("/ssid");
ESPFlashString password_eeprom("/passwd");

int relay_status = 0;
bool AP_On = (network.ssid == 0 && network.password == 0);

int connect_to_wifi(const Network& network){
  WiFi.begin(network.ssid, network.password);
  int counter = 0;
  while (WiFi.status() != WL_CONNECTED && counter < 12){ 
    counter++;
    delay(1000);
    Serial.print('*');
  }
  if(WiFi.status() == WL_CONNECTED){
    return 1;
  }else{
    return 0;
  }
}

void setup(){
  Serial.begin(9600);
  delay(10);
  pinMode(RELAY_COM, OUTPUT);
  pinMode(LED_STATE_GREEN, OUTPUT);
  pinMode(LED_STATE_RED, OUTPUT);
  pinMode(RESET_BUTTON, INPUT);
  delay(10);
  network.ssid = ssid_eeprom.get();
  network.password = password_eeprom.get();
  
  relay_status = relay_status_eeprom.get();
     Serial.print("Relay status AT START: ");
     Serial.println(relay_status);
  if(relay_status != 1){ //first ever turning on
    relay_status_eeprom.set(0);
    relay_status = relay_status_eeprom.get();
  }
  turn_relay(relay_status);
  change_state_led(255,20);

  AP_On = (network.ssid == 0 && network.password == 0);
  delay(1000);
  if(AP_On == true){
    change_state_led(255,20);
    delay(1000);
    WiFi.mode(WIFI_AP); //Changing Wifi mode to AccessPoint
    WiFi.softAP("SmartSocket");
    server.on("/", rootHandler);
    server.on("/process-info", processInfoHandler); 
    server.begin();
    Serial.println("Web server started!");
  }else{
    delay(1000); // Wait for one second before trying to connect
    WiFi.mode(WIFI_STA);  //Changing Wifi mode to Station
    Serial.println("Known networks list is not empty! Trying to connect to a known network...");
    if(connect_to_wifi(network) == 1){
        Serial.println("Connected to the WiFi network!");
        Serial.println(network.ssid);
        Serial.println(WiFi.localIP());
        delay(1000);    
        
        client.setServer(MQTT_SERVER_ADDRESS, MQTT_PORT);
        client.setCallback(callback);
        
        while (!client.connected()) {
          Serial.println("Connecting to MQTT...");
      
          if(client.connect("ESP8266Client", MQTT_USER, MQTT_PASSWORD )) {
            Serial.println("Connected to the MQTT Broker.");  
          }else {        
            Serial.print("Connectiong to MQTT failed. Error code: ");
            Serial.print(client.state());
            delay(2000);
          }
          String topic = String("socket/") + String(DEVICE_UID);
          client.subscribe((char*) topic.c_str());
        
        }
    }else{
        Serial.println("Connection failed!");  
    }
    if(relay_status == 0){
      change_state_led(255,0);
    }else if(relay_status == 1){
      change_state_led(0,45);
    }
  }
  
  Serial.println("MCU started successfully!");

}

void loop() {
    if(AP_On == true){
     server.handleClient();
    }
    client.loop();
    reset_esp();
}

void rootHandler() {
  server.send(200, "text/html", index_page);
}

void processInfoHandler(){
 if(server.args()== 2){
  String input_ssid = server.arg("input_ssid"); 
  String input_pswd = server.arg("input_pswd"); 

  Serial.println("Information read from HTML form!");
  Serial.print("SSID: ");
  Serial.println(input_ssid);

  Serial.print("Password: ");
  Serial.println(input_pswd);
  
  ssid_eeprom.set(input_ssid);
  network.ssid = ssid_eeprom.get();
  delay(200);
  password_eeprom.set(input_pswd);
  network.ssid = password_eeprom.get();
  
  delay(1000);
  server.send(200, "text/html", index_page);
  delay(1000);
  setup();
 }else{
  Serial.println("None or more than 2 arguments were given.");
 }
}

/*
  Turned off state (RED)       = 255, 0,  0
  Not connected state (ORANGE) = 255, 20, 0
  Turned on state (GREEN)      =   0, 45, 0
*/

void change_state_led(int R_VALUE, int G_VALUE){
  analogWrite(LED_STATE_RED, R_VALUE);
  analogWrite(LED_STATE_GREEN, G_VALUE);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("-----------------------");
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
 
  Serial.print("Message:");
  
  String payload_string = "";

  for (int i = 0; i < length; i++) {
    payload_string += String((char)payload[i]);
  }

  Serial.print(payload_string);
  Serial.println();
  Serial.println("-----------------------");

 if(payload_string == "GET"){
  String answer = "DEVICE-GET " + String(relay_status);
  char topicCopy [strlen(topic)+1];
  strcpy (topicCopy, topic);
  client.publish(topicCopy, (char*) answer.c_str());
 }
 
 if(payload_string.substring(0, 3) == "SET"){
  bool flag_to_set = false;
   if(payload_string.endsWith("1")){
    turn_relay(HIGH);
    flag_to_set = true;
  }else if(payload_string.endsWith("0")){
    flag_to_set = true;
    turn_relay(LOW);
  }
  String answer = "";
  if(flag_to_set == true){
    answer = "DEVICE-SET " + String(relay_status);
  }else{
    answer = "DEVICE-SET ERROR";
   }
   char topicCopy [strlen(topic)+1];
   strcpy (topicCopy, topic);
   client.publish(topicCopy, (char*) answer.c_str());
  
 }
 
}

void reset_esp(){
  int buttonState=0;
  int counter = 0;
  buttonState=digitalRead(RESET_BUTTON);
  if(buttonState == HIGH){
      counter++;
      if(counter == 1){
         ssid_eeprom.reset();
         password_eeprom.reset();
         relay_status_eeprom.set(0);
         network.ssid = ssid_eeprom.get();
         network.password = password_eeprom.get();
         relay_status = relay_status_eeprom.get();
         ESP.restart();
         delay(200);
       }
       
  }else{
    buttonState=0;
  }
}

void turn_relay(bool state){
  relay_status_eeprom.set(state);
  relay_status = relay_status_eeprom.get();
  digitalWrite(RELAY_COM, state);
  if(relay_status == 0){
    change_state_led(255,0);
  }else if(relay_status == 1){
    change_state_led(0,45);
  }
}
