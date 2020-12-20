#include <ESP8266WiFi.h>
#include <vector>

#define KNOWN_NETWORKS_MAX 5

typedef struct network_t{
    String ssid;
    String password;
} Network;

std::vector<Network> known_networks;

WiFiClient client;
WiFiServer server(80);

bool AP_On = (known_networks.size() == 0);

void setup() {
  Serial.begin(9600);
  if(AP_On){
    WiFi.softAP("SmartSocket");
    server.begin();
  }
  
  /* Debug */
  Serial.println("MCU Started!");
  Serial.println(WiFi.softAPIP());
  Serial.println(AP_On);
  /* end Debug */
}

void loop() {
    if(AP_On){
      client = server.available();  
      String html ="<!DOCTYPE html><html> <head> <title>SmartSocket - Welcome</title> <style> .center{ width:250px; height:350px; margin: 0 auto; } </style> </head> <body> <div class=\"center\"> <h2>Welcome!</h2> <h3>Enter your WiFi's credentials</h3> <form method=\"post\"> <label for=\"label_ssid\">SSID:</label><br> <input type=\"text\" id=\"input_ssid\" name=\"input_ssid\"><br> <label for=\"label_pswd\">Password:</label><br> <input type=\"text\" id=\"input_pswd\" name=\"input_pswd\"><br><br> <input type=\"submit\" value=\"Submit\"> </form> </div> </body> </html>";
      client.println(html);
    }
}
