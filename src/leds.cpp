#include "leds.h"

TaskHandle_t Leds::redLedTask,Leds::greenLedTask;

unsigned int Leds::redOn = 100;
unsigned int Leds::redOff = 100;
unsigned int Leds::greenOn = 100;
unsigned int Leds::greenOff = 100;

Leds::BTSTATUS Leds::btStatus;
Leds::WIFISTATUS Leds::wifiStatus;
Leds::GPSSTATUS Leds::gpsStatus; 

extern Logger logger;

void Leds::manageRedLed(void * pvParameters){
  unsigned int btLedOn,btLedOff,wifiLedOn,wifiLedOff;
  for(;;){    
    switch (btStatus) {
      case bt_off:
        btLedOn = 1;
        btLedOff = 999;
        break;
      case bt_on:
        btLedOn = 100;
        btLedOff = 100;
        break;
      case bt_connected:
        btLedOn = 999;
        btLedOff = 1;
        break;
    }
    switch (wifiStatus) {
        case wifi_off:
          wifiLedOn = 1;
          wifiLedOff = 999;
          break;
        case wifi_on:
          wifiLedOn = 100;
          wifiLedOff = 100;
          break;
        case wifi_connected:
          wifiLedOn = 999;
          wifiLedOff = 1;
          break;
    }
    redOn = btLedOn;
    redOff = btLedOff;
    digitalWrite(RED_LED, HIGH);
    delay(redOn);
    digitalWrite(RED_LED, LOW);
    delay(redOff);
  } 
}

void Leds::manageGreenLed(void * pvParameters){
  for(;;){
    switch(gpsStatus) {
      case searching:
        greenOn=greenOff=100;
        break;
      case locked:
        greenOn=greenOff=1000;
    }    
    digitalWrite(GREEN_LED, HIGH);
    delay(greenOn);
    digitalWrite(GREEN_LED, LOW);
    delay(greenOff);
  }
}
  
void Leds::setup() {
    pinMode(RED_LED,OUTPUT);
    pinMode(GREEN_LED,OUTPUT);
    xTaskCreate(Leds::manageRedLed,"redLed",1024,NULL,10,&redLedTask);
    xTaskCreate(Leds::manageGreenLed,"greenLed",1024,NULL,10,&greenLedTask);  
    logger.print("LED+"); 
}