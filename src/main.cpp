#include "main.h"
#include <esp_bt.h>
#include <esp_wifi.h>
#include <esp_sleep.h>

#define USE_WIFI false
#define INT_PIN 34

Logger logger;
Leds leds;
BLEData bleData;

bool debug = false;
bool isModemSleepOn = false;
bool isGoToSleep = false;
bool isWakeUp = false;
bool motion = false;
String command;
TaskHandle_t peripheralTask;
MPU9250_WE mpu;

void wifi_off() {
    WiFi.disconnect(true,false);
    WiFi.mode(WIFI_OFF);
    //leds.wifiStatus=Leds::WIFISTATUS::wifi_off;
    logger.print("NET-");
}

bool wifi_connect() {
    const char *found_ssid = NULL;
    int n = 0;

    for (int i = 0; i < 3; i++) {
        n = WiFi.scanNetworks();
        if (n > 0) {
            break;
        }
        delay(250);
    }

    for (int i = 0; i < n; ++i) {
        int j = 0;
        while (WIFI_CREDENTIALS[j][0] != NULL) {
            if (WiFi.SSID(i) == WIFI_CREDENTIALS[j][0]) {
                found_ssid = WIFI_CREDENTIALS[j][0];
                const char *passphrase = WIFI_CREDENTIALS[j][1];
                WiFi.begin(found_ssid, passphrase);
                break;
            }
            j++;
        }
    }

    if (found_ssid == NULL) {
        logger.println("No known WiFi found.");
        wifi_off();
        return false;
    }

    logger.printfln("Connecting to WiFi: %s ...", found_ssid);
    WiFi.mode(WIFI_STA);
    leds.wifiStatus=Leds::WIFISTATUS::wifi_on;
    int tries = 50;
    while (WiFi.status() != WL_CONNECTED && tries > 0) {
        delay(250);
        tries--;
    }
    if (tries == 0) {
        logger.println("Failed to connect to WiFi!");
        wifi_off();
        return false;
    }
    
    leds.wifiStatus=Leds::WIFISTATUS::wifi_connected;
    logger.print("NET+");
    return true;
}

void wifi_setup() {
    if (USE_WIFI) wifi_connect();
    else wifi_off();
}

void modem_sleep() {
    ble_stop();
    btStop();
    //WiFi.disconnect();
    //WiFi.setSleep(true);
    //WiFi.mode(WIFI_OFF);
    //setCpuFrequencyMhz(40);
    isModemSleepOn = true;
}

void modem_awake() {
    isModemSleepOn = false;
    //setCpuFrequencyMhz(240);
    btStart();
    ble_setup();
}

void check_incoming_commands() {
    command = ble_uart_receive();
    if (Serial.available()>0) command = Serial.readStringUntil('\n');
    // command = logger.udpReceive();
    if (command.length()==0) return;
    command.trim();
    if (command == "d" || command == "gps") {
        debug = !debug;
        logger.printfln("Debug %s...", debug ? "ON" : "OFF");
    } 
    else if (command == "" || command == "reset") {
        logger.println("Restarting ESP...");
        delay(1000);
        ESP.restart();
    } 
    else if (command == "s" || command == "sleep") {
        logger.println("Entering Modem Sleep...");
        modem_sleep();
    }   
    else if (command == "u" || command == "upload") {
        logger.println("Uploading Log...");
        //TODO implement upload
    }
}

void managePeripheral(void * pvParameters) {
    unsigned long last = millis();
    unsigned long lastlog = last;
    unsigned int sleep_motion_counter = 0;
    unsigned int wake_motion_counter = 0;

    for(;;) {
        delay(50);
        unsigned long now = millis();
        if ((now-last) < TICK_INTERVAL) continue;
        last=now;
        //stuff to do every TICK_INTERVAL
        if (motion) {
            byte source = mpu.readAndClearInterrupts();
            logger.print("I");
            if(mpu.checkInterrupt(source, MPU9250_WOM_INT)) {
                logger.print("M");
            }
            motion = false;
            mpu.readAndClearInterrupts();             
        }
        if ((now-lastlog) >= LOG_INTERVAL) { //do checks every 1 second
            lastlog = now;
            //stuff to do every LOG_INTERVAL

//            if (gps.location.isValid() && isLocUpdated) {
//                char record[80];
//                snprintf(record, sizeof(record), TIMESTAMP_FORMAT ";%.6f;%.6f;%.2f;%.2f;%.2f;%d",
//                        TIMESTAMP_ARGS,
//                        gps.location.lat(), gps.location.lng(), gps.altitude.meters(), gps.speed.kmph(), gps.hdop.hdop(), gps.satellites.value());
//                logger.println(record);
//            }
//            else logger.print(".");
            logger.print("L");

            if (!isModemSleepOn) ble_update(&bleData,&mpu);

            if (sleep_motion_counter > SLEEP_TRIGGER_COUNT) {
                sleep_motion_counter = 0;
                wake_motion_counter = 0;
                if(!isModemSleepOn && !isGoToSleep) {
                    logger.println("Entering Modem Sleep Mode");
                    isGoToSleep = true;
                }
            }
            if (wake_motion_counter > WAKE_TRIGGER_COUNT) {
                sleep_motion_counter = 0;
                wake_motion_counter = 0;
                if (isModemSleepOn && !isWakeUp) {  
                    logger.println("Exiting Modem Sleep Mode");
                    isWakeUp = true;
                }
            }
        }
    }
}

void motionISR() {
  motion = true;
}

void mpu_setup() {
    Wire.begin();
    if(mpu.init()) {
        mpu.setSampleRateDivider(5);
        mpu.setAccRange(MPU6500_ACC_RANGE_2G);
        mpu.enableAccDLPF(true);
        mpu.setAccDLPF(MPU6500_DLPF_6);
        mpu.setIntPinPolarity(MPU6500_ACT_HIGH);
        mpu.enableIntLatch(true);
        mpu.enableClearIntByAnyRead(false);
        mpu.enableInterrupt(MPU6500_WOM_INT);
        mpu.setWakeOnMotionThreshold(5);
        mpu.enableWakeOnMotion(MPU9250_WOM_ENABLE, MPU9250_WOM_COMP_ENABLE);
        attachInterrupt(digitalPinToInterrupt(INT_PIN), motionISR, RISING);
        xTaskCreate(managePeripheral,"MPU",8192,NULL,1,&peripheralTask);
        logger.print("MPU+");
    }
    else {
        logger.print("MPU-");
    }
}

void setup() {
    ota_setup();
    wifi_setup();    
    //leds.setup();
    ble_setup();
    mpu_setup();
    //logger.udpListen();
    logger.println("UP");
}

void loop() {
    check_incoming_commands();
    if (isGoToSleep) {
        modem_sleep();
        isGoToSleep = false;
    }
    if (isWakeUp) {
        modem_awake();
        isWakeUp = false;
    }
}
