#include <WifiUdp.h>
#include <Wire.h>
#include <MPU9250_WE.h>
#include "logging.h"
#include "ble.h"
#include "bledata.h"
#include "wifi_credentials.h"
#include "ota.h"
#include "leds.h"

#define TICK_INTERVAL 500
#define LOG_INTERVAL 1000
#define TIMESTAMP_FORMAT "%04d-%02d-%02dT%02d:%02d:%02dZ"
#define TIMESTAMP_ARGS gps.date.year(), gps.date.month(), gps.date.day(), gps.time.hour(), gps.time.minute(), gps.time.second()
#define SLEEP_TRIGGER_SPEED 4
#define SLEEP_TRIGGER_COUNT 10
#define WAKE_TRIGGER_COUNT 30
#define MAX_HDOP 2.0