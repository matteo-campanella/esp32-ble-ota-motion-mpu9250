#include <Arduino.h>
#include "logging.h"

#define RED_LED 22
#define GREEN_LED 23

class Leds{
    private:
        static TaskHandle_t greenLedTask,redLedTask;
        static unsigned int redOn,redOff,greenOff,greenOn;
    public:
        enum WIFISTATUS {
            wifi_on,
            wifi_connected,
            wifi_off,
        };
        enum BTSTATUS {
            bt_on,
            bt_connected,
            bt_off
        };
        enum GPSSTATUS {
            searching,
            locked
        };
        static WIFISTATUS wifiStatus;
        static BTSTATUS btStatus;
        static GPSSTATUS gpsStatus;
        static void setup();
        static void manageRedLed(void *);
        static void manageGreenLed(void *);
};