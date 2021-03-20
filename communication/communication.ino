#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <WiFiClient.h>

#include "WebPages.h"

#include <vector>

#define LED_STATE_RED       D1
#define LED_STATE_GREEN     D2

#define RELAY_COM           D5

#define RESET_BUTTON        D7

#define MQTT_SERVER_ADDRESS ""
#define MQTT_PORT 0
#define MQTT_USER ""
#define MQTT_PASSWORD ""

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

ESP8266WebServer server(80);

WiFiClient wifi_client;
PubSubClient client(wifi_client);

bool AP_On = (known_networks.size() == 0);

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
  pinMode(RELAY_COM, OUTPUT);
  pinMode(LED_STATE_GREEN, OUTPUT);
  pinMode(LED_STATE_RED, OUTPUT);
  pinMode(RESET_BUTTON, INPUT);
  AP_On = (known_networks.size() == 0);
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
    
    /* Debug */
    Serial.println("Known networks list is not empty! Trying to connect to a known network...");
    /* end Debug */
    if(connect_to_wifi(known_networks[0]) == 1){
      Serial.println("Connected to the WiFi network!");
      //change_state_led(0,45);
      Serial.println(known_networks[0].ssid);
      Serial.println(WiFi.localIP());

      delay(1000);    
      
      client.setServer(MQTT_SERVER_ADDRESS, MQTT_PORT);
      client.setCallback(callback);
      
      while (!client.connected()) {
      Serial.println("Connecting to MQTT...");
  
      if(client.connect("ESP8266Client", MQTT_USER, MQTT_PASSWORD )) {
        Serial.println("Connected to the MQTT Broker.");  
      }else {        
        change_state_led(255,0);
        Serial.print("Connectiong to MQTT failed. Error code: ");
        Serial.print(client.state());
        delay(2000);
      }
      client.subscribe("esp/test");
      }


    }else{
      Serial.println("Connection failed!");  
      change_state_led(255,0);
    }
  }
  
  /* Debug */
  Serial.println("MCU started successfully!");
  //change_state_led(0,45);
  /* end Debug */
}

void loop() {
    if(AP_On == true){
     server.handleClient();
      /*change_state_led(255,0);
    delay(1000);
    change_state_led(255,20);
    delay(1000);
    change_state_led(0,45);
    delay(1000);*/
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

  known_networks.push_back(Network(input_ssid, input_pswd));
  delay(1000);
  server.send(200, "text/html", index_page);
  delay(1000);
  setup();
 }else{
  Serial.println("None or more than 2 arguments were given.");
  //TODO: Redirect and valid info filter.
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
  
  String payload_string;

  for (int i = 0; i < length; i++) {
    payload_string += String((char)payload[i]);
  }
 Serial.print(payload_string);

 if(payload_string == "1"){
    turn_relay(HIGH);
 }else if(payload_string == "0"){
    turn_relay(LOW);
  }
 
  Serial.println();
  Serial.println("-----------------------");
 
}

void reset_esp(){
  int buttonState=0;
  int counter = 0;
  buttonState=digitalRead(RESET_BUTTON);
  if(buttonState == HIGH){
      counter++;
      if(counter == 1){
        //Serial.println("BUTTOONN");
         known_networks.clear();
         ESP.restart();
         delay(200);
       }
       
  }else{
    buttonState=0;
  }
}

void turn_relay(bool state){
  digitalWrite(RELAY_COM, state);
}
