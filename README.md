# esp32-gps-ble

using the template developed for gps alt and speed monitor to build a LOW ENERGY sensor to use in conjunction with sim equipped GPS (on bike)

the idea is to have this circuit to act as a switch which will stay dormient until an interrupt is generated from MPU9250 because of motion; at this point the switch will turn the GPS transmitter on and the position will be monitored on Ruhaivik.

after the motion ends (after a guardtime / ble interaction) the switch will go off again and the circuit back to dormient status again

low consumption is of the utter importance

#connection pins

BMI160 | ESP32
---|---
GND | GND
VCC | VCC
INT1 | G34
INT2 | ???
SDA | G21
SCL | G22