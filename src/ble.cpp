#include <CircularBuffer.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Arduino.h>
#include <MPU9250_WE.h>

#include "ble.h"
#include "logging.h"
#include "leds.h"
#include <string>

BLEServer* pServer = NULL;
BLECharacteristic* pLocCharacteristic = NULL;
BLECharacteristic* pSpeedCharacteristic = NULL;
BLECharacteristic* pAltCharacteristic = NULL;
BLECharacteristic* pSettingsCharacteristic = NULL;
BLECharacteristic *pTxCharacteristic, *pRxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;
String inCommand;
String tmp;
char outBuffer[80];

extern Logger logger;
extern Leds leds;

#define BLE_DEVICE_NAME "bleMotion"
#define UART_SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define UART_CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define UART_CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

#define GPS_SERVICE_UUID        "f7cfe716-cf46-440c-b763-2b10f181051e"
#define LOC_CHARACTERISTIC_UUID "0a75ba9e-7cf2-490b-9811-f16af03730db"
#define SPD_CHARACTERISTIC_UUID "f37a98dd-7da6-4578-adae-9dbd481b7034"
#define ALT_CHARACTERISTIC_UUID "0538d525-011e-450b-a9e4-920010a0cf63"
#define SETTINGS_CHARACTERISTIC_UUID "8ec19f5a-d47a-49f8-b91f-cd8c9acd1529"


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      BLEDevice::startAdvertising();
      leds.btStatus = Leds::BTSTATUS::bt_connected;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      leds.btStatus = Leds::BTSTATUS::bt_on;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      if (pCharacteristic == pRxCharacteristic) {
        inCommand = pCharacteristic->getValue().c_str();
      }
      else if (pCharacteristic == pSettingsCharacteristic) {
        logger.print("settings write");
      }
    }
};

void ble_setup() {
  // Create the BLE Device
  BLEDevice::init(BLE_DEVICE_NAME);

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  //UART Service
  BLEService *pUARTService = pServer->createService(UART_SERVICE_UUID);
  pTxCharacteristic = pUARTService->createCharacteristic(
										UART_CHARACTERISTIC_UUID_TX,
										BLECharacteristic::PROPERTY_NOTIFY
									);
  pTxCharacteristic->addDescriptor(new BLE2902());
  pRxCharacteristic = pUARTService->createCharacteristic(
											UART_CHARACTERISTIC_UUID_RX,
											BLECharacteristic::PROPERTY_WRITE
										);
  pRxCharacteristic->setCallbacks(new MyCallbacks());     
  pUARTService->start();

  //GPS Service
  BLEService *pGpsService = pServer->createService(GPS_SERVICE_UUID);
  pLocCharacteristic = pGpsService->createCharacteristic(
                      LOC_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  pLocCharacteristic->addDescriptor(new BLE2902());

  pSpeedCharacteristic = pGpsService->createCharacteristic(
                      SPD_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  pSpeedCharacteristic->addDescriptor(new BLE2902());

  pAltCharacteristic = pGpsService->createCharacteristic(
                      ALT_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  pAltCharacteristic->addDescriptor(new BLE2902());

  pSettingsCharacteristic = pGpsService->createCharacteristic(
                      SETTINGS_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE
                    );
  //pSettingsCharacteristic->addDescriptor(new BLE2902());  

  pGpsService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(GPS_SERVICE_UUID);
  pAdvertising->addServiceUUID(UART_SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  BLEDevice::startAdvertising();
  Leds::btStatus = Leds::BTSTATUS::bt_on;
  logger.print("BLE+");
}

void ble_uart_send(const char *message) {
    if (deviceConnected) {
      pTxCharacteristic->setValue((uint8_t*)message,strlen(message));
      pTxCharacteristic->notify();
		  delay(10); // bluetooth stack will go into congestion, if too many packets are sent
	  }
}

String ble_uart_receive() {
  tmp = "";
  if (inCommand.length()>0) {
    tmp = inCommand;
    inCommand = "";
  }
  return tmp;
}

void ble_update(BLEData *data) {
  if (deviceConnected) {
/*       if (gps->location.isValid()) {
      snprintf(outBuffer, sizeof(outBuffer), "%.3f;%.3f;%.2f;%d",
        gps->location.lat(), gps->location.lng(), gps->hdop.hdop(), gps->satellites.value());
      pLocCharacteristic->setValue(outBuffer);
      pLocCharacteristic->notify();
      delay(10);
    }
    if (gps->speed.isValid()) {
      snprintf(outBuffer, sizeof(outBuffer),"%.2f;%.2f;%.2f",data->min_speed,data->speed,data->max_speed);
      pSpeedCharacteristic->setValue(outBuffer);
      pSpeedCharacteristic->notify(); 
      delay(10); 
    }
    if (gps->altitude.isValid()) {
      snprintf(outBuffer, sizeof(outBuffer),"%.2f;%.2f;%.2f",data->min_alt,data->alt,data->max_alt);
      pAltCharacteristic->setValue(outBuffer);
      pAltCharacteristic->notify();
      delay(10);
    } */
  }
  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
      delay(500); // give the bluetooth stack the chance to get things ready
      pServer->startAdvertising(); // restart advertising
      oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
      oldDeviceConnected = deviceConnected;
  }
}

void ble_stop() {
  BLEDevice::deinit();
  leds.btStatus = Leds::BTSTATUS::bt_off;
  logger.print("BLE-");
}