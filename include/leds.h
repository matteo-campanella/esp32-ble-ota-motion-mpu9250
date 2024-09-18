#include <Arduino.h>
#include "logging.h"

#define RED_LED 23

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
        enum MOTIONSTATUS {
            off,
            detected,
            on
        };
        static WIFISTATUS wifiStatus;
        static BTSTATUS btStatus;
        static MOTIONSTATUS motionStatus;
        static void setup();
        static void manageRedLed(void *);
};