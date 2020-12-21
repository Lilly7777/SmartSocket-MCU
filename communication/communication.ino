#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>

#include "WebPages.h"

#include <vector>

#define LED_STATE_RED        5    // D1
#define LED_STATE_GREEN      4    // D2

#define RELAY_COM           14    // D5

#define RESET_BUTTON        13    // D7

#define KNOWN_NETWORKS_MAX   5

typedef struct network_t{
    String ssid;
    String password;
} Network;

std::vector<Network> known_networks;

ESP8266WebServer server(80);

bool AP_On = (known_networks.size() == 0);  //TODO: Add more cases when AP will be on

void setup(){
  Serial.begin(9600);
  if(AP_On){
    WiFi.softAP("SmartSocket");
    server.on("/", rootHandler);
    server.begin();
    Serial.println("Web server started!");

  }else{
    delay(1000); // Wait for one second before trying to connect
    WiFi.softAPdisconnect(true); //Make sure AP is turned off
    
    /* Debug */
    Serial.println("Known networks list is not empty! Trying to connect to a known network...");
    /* end Debug */
    
  }
  
  /* Debug */
  Serial.println("MCU started successfully!");
  Serial.println(WiFi.softAPIP());
  Serial.println(AP_On);
  /* end Debug */
}

void loop() {
    if(AP_On){
     server.handleClient();
    }
}

void rootHandler() {
  server.send(200, "text/html", index_page);
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
