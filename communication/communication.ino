#include <ESP8266WiFi.h>

#include <vector>

#define LED_STATE_RED   16    //D0
#define LED_STATE_GREEN 5   //D1
#define LED_STATE_BLUE  4    //D2  

#define KNOWN_NETWORKS_MAX 5

typedef struct network_t{
    String ssid;
    String password;
} Network;

std::vector<Network> known_networks;

WiFiServer server(80);

bool AP_On = (known_networks.size() == 0);  //TODO: Add more cases when AP will be on

void setup(){
  Serial.begin(9600);
  
  pinMode(LED_STATE_RED, OUTPUT);
  pinMode(LED_STATE_GREEN, OUTPUT);
  pinMode(LED_STATE_BLUE, OUTPUT);
  
  if(AP_On){
    WiFi.softAP("SmartSocket");
    server.begin();
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
  Serial.println(is_connected);
  /* end Debug */
}

void loop() {
    if(AP_On){
      WiFiClient client = server.available();  
      String html ="<!DOCTYPE html><html> <head> <title>SmartSocket - Welcome</title> <style> .center{ width:250px; height:350px; margin: 0 auto; } </style> </head> <body> <div class=\"center\"> <h2>Welcome!</h2> <h3>Enter your WiFi's credentials</h3> <form method=\"post\"> <label for=\"label_ssid\">SSID:</label><br> <input type=\"text\" id=\"input_ssid\" name=\"input_ssid\"><br> <label for=\"label_pswd\">Password:</label><br> <input type=\"text\" id=\"input_pswd\" name=\"input_pswd\"><br><br> <input type=\"submit\" value=\"Submit\"> </form> </div> </body> </html>";
      client.println(html);
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
