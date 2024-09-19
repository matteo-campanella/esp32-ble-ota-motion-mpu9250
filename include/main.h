#include <CircularBuffer.hpp>
#include <WifiUdp.h>
#include <Wire.h>
#include <BMI160Gen.h>
#include "logging.h"
#include "ble.h"
#include "bledata.h"
#include "wifi_credentials.h"
#include "ota.h"
#include "leds.h"
#include <movingAvg.h>

#define TICK_INTERVAL 500
#define LOG_INTERVAL 1000
#define SLEEP_TRIGGER_COUNT 360
#define WAKE_TRIGGER_COUNT 30
#define USE_WIFI false
#define INT_PIN 34
#define ADC_PIN 35
#define ADC_VOLT_COEFF 1678
#define SWITCH_PIN 33 //TODO put right value
