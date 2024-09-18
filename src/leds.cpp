#include "leds.h"

TaskHandle_t Leds::redLedTask,Leds::greenLedTask;

unsigned int Leds::redOn = 100;
unsigned int Leds::redOff = 100;

Leds::BTSTATUS Leds::btStatus;
Leds::WIFISTATUS Leds::wifiStatus;
Leds::MOTIONSTATUS Leds::motionStatus; 

extern Logger logger;

void Leds::manageRedLed(void * pvParameters){
  unsigned int btLedOn,btLedOff,wifiLedOn,wifiLedOff,motionLedOn,motionLedOff;
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
    switch (motionStatus) {
      case on:
        motionLedOn = 999;
        motionLedOff = 1;
        break;
      case off:
        motionLedOn = 1;
        motionLedOff = 999;
        break;
      case detected:
        motionLedOn = 50;
        motionLedOff = 50;
      break;
    }
    redOn = motionLedOn;
    redOff = motionLedOff;
    digitalWrite(RED_LED, HIGH);
    delay(redOn);
    digitalWrite(RED_LED, LOW);
    delay(redOff);
  } 
}
  
void Leds::setup() {
    pinMode(RED_LED,OUTPUT);
    xTaskCreate(Leds::manageRedLed,"redLed",1024,NULL,10,&redLedTask); 
    logger.print("LED+"); 
}