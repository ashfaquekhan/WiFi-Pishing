#include <ESP8266WiFi.h>
#include <Wire.h>
#include "SSD1306.h"
#include <ezButton.h>
SSD1306 display(0x3c, 4, 5); 

const int SHORT_PRESS_TIME = 500; 
const int LONG_PRESS_TIME  = 2000;

ezButton button(2);  

unsigned long pressedTime  = 0;
unsigned long releasedTime = 0;
boolean bt = false;
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
    Serial.println("A long press is detected");
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
    display.drawString(6, 50,"/");
    display.drawString(10, 50, String(n));
    display.display();
}
void loop()
{
  disp();
}
