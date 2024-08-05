#include <MPU9250_WE.h>
#include <string>
#include "bledata.h"

void ble_setup();
void ble_update(BLEData *);
void ble_uart_send(const char *);
String ble_uart_receive();
void ble_stop();