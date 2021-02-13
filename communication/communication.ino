#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>

#include "WebPages.h"

#include <vector>

#define LED_STATE_RED        5    // D1
#define LED_STATE_GREEN      4    // D2

#define RELAY_COM           14    // D5

#define RESET_BUTTON        13    // D7

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
  AP_On = (known_networks.size() == 0);
  delay(1000);
  if(AP_On == true){
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
      Serial.println(known_networks[0].ssid);
      Serial.println(WiFi.localIP());
    }else{
      Serial.println("Connection failed!");  
    }
  }
  
  /* Debug */
  Serial.println("MCU started successfully!");
  /* end Debug */
}

void loop() {
    if(AP_On == true){
     server.handleClient();
    }
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

void change_state_led(int R_VALUE, int G_VALUE, int B_VALUE){
  analogWrite(R_VALUE, R_VALUE);
  analogWrite(G_VALUE, G_VALUE);
  analogWrite(B_VALUE, B_VALUE);
}
