#include <WifiUdp.h>
#include <Wire.h>
#include <MPU9250_WE.h>
#include "logging.h"
#include "ble.h"
#include "bledata.h"
#include "wifi_credentials.h"
#include "ota.h"
#include "leds.h"
#include <movingAvg.h>

#define TICK_INTERVAL 500
#define LOG_INTERVAL 1000
#define SLEEP_TRIGGER_COUNT 120
#define WAKE_TRIGGER_COUNT 30
#define USE_WIFI false
#define INT_PIN 34
#define ADC_PIN 35
#define ADC_VOLT_COEFF 12
#define MOTION_TRESHOLD 3
#define SWITCH_PIN 23 //TODO put right value