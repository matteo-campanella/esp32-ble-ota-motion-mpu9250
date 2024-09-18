#include "main.h"
#include <esp_bt.h>
#include <esp_wifi.h>
#include <esp_sleep.h>

Logger logger;
Leds leds;
BLEData bleData;

bool debug = false;
bool isModemSleepOn = false;
bool isGoToSleep = false;
bool isWakeUp = false;
bool motion = false;
bool motionWakeUp = false;
bool switchStatus = false;
movingAvg voltage(10),temperature(10);

String command;
TaskHandle_t BMITask,commTask,sensorsTask;

void wifi_off() {
    WiFi.disconnect(true,false);
    WiFi.mode(WIFI_OFF);
    leds.wifiStatus=Leds::WIFISTATUS::wifi_off;
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

void esp_deep_sleep() {
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_34,0);
    logger.println("Entering Deep Sleep...");
    delay(1000);
    esp_deep_sleep_start();
}

void check_incoming_commands() {
    command = ble_uart_receive();
    if (Serial.available()>0) command = Serial.readStringUntil('\n');
    // command = logger.udpReceive();
    if (command.length()==0) return;
    command.trim();
    if (command == "d" || command == "debug") {
        debug = !debug;
        logger.printfln("Debug %s...", debug ? "ON" : "OFF");
    } 
    else if (command == "r" || command == "reset") {
        logger.println("Restarting ESP...");
        delay(1000);
        ESP.restart();
    } 
    else if (command == "s" || command == "sleep") {
        logger.println("Entering Sleep...");
        esp_deep_sleep();
    }   
    else if (command == "u" || command == "upload") {
        logger.println("Uploading Log...");
        //TODO implement upload
    }
    else if (command == "m" || command == "dump") {
        logger.println("Dumping Data...");
        logger.printf("V: %d\n",bleData.voltage);
        logger.println("End Dumping Data");
    }   
}

void manageSensors(void * pvParameters) {
    voltage.begin();
    for(;;) {
        voltage.reading((analogRead(ADC_PIN)*ADC_VOLT_COEFF)/4095);
        bleData.voltage = voltage.getAvg();
        delay(200); 
    }
}

void manageComms(void * pvParameters) {
    for(;;) {
        if (!isModemSleepOn) ble_update(&bleData);
        delay(2000);
    }
}

void motionISR() {
  motion = true;
}

void manageBMI(void * pvParameters) {
    unsigned long last = millis();
    unsigned long lastlog = last;
    unsigned int sleep_motion_counter = 0;
    unsigned int wake_motion_counter = 0;
    temperature.begin();

    for(;;) {
        delay(50);
        unsigned long now = millis();
        if ((now-last) < TICK_INTERVAL) continue;
        last=now;
        //stuff to do every TICK_INTERVAL
        if (motion) {
            logger.print("I");
            Leds::motionStatus = Leds::MOTIONSTATUS::detected;
            sleep_motion_counter=0;
            wake_motion_counter++;
            motion = false;             
        }
        else {
            Leds::motionStatus = switchStatus?Leds::MOTIONSTATUS::on:Leds::MOTIONSTATUS::off;
            wake_motion_counter=0;
            if (switchStatus) sleep_motion_counter++;
            else sleep_motion_counter+=3; //if it's not on make it go to sleep faster
        }        

        if ((now-lastlog) >= LOG_INTERVAL) { //do checks every 1 second
            lastlog = now;
            //stuff to do every LOG_INTERVAL

            if (sleep_motion_counter >= SLEEP_TRIGGER_COUNT) {
                sleep_motion_counter = 0;
                wake_motion_counter = 0;
                if (!isGoToSleep) isGoToSleep = true;
/*                 if(!isModemSleepOn && !isGoToSleep) {
                    logger.println("Entering Modem Sleep Mode");
                    isGoToSleep = true;
                } */
            }
            if (wake_motion_counter >= WAKE_TRIGGER_COUNT) {
                sleep_motion_counter = 0;
                wake_motion_counter = 0;
                if (!isWakeUp) isWakeUp = true;
/*                 if (isModemSleepOn && !isWakeUp) {  
                    logger.println("Exiting Modem Sleep Mode");
                    isWakeUp = true;
                } */
            }
            logger.print(".");
        }
    }
}       

void bmi160_setup() {
    Wire.begin();
    if (BMI160.begin(BMI160GenClass::I2C_MODE, Wire, 0x69, INT_PIN)) {    
        BMI160.attachInterrupt(motionISR);
        BMI160.setIntMotionEnabled(true);
        xTaskCreate(manageBMI,"BMI",8192,NULL,1,&BMITask);
        logger.print("BMI+");
    }
    else {
        logger.print("BMI-");
    }
}

void switchOn() {
    logger.println("Switching ON");
    digitalWrite(SWITCH_PIN,HIGH);
    switchStatus = true;
    Leds::motionStatus = Leds::MOTIONSTATUS::on;
}

void switchOff() {
    logger.println("Switching Off");
    digitalWrite(SWITCH_PIN,LOW);
    switchStatus = false;
    Leds::motionStatus = Leds::MOTIONSTATUS::off;
}

void switch_setup(bool status) {
    pinMode(SWITCH_PIN,OUTPUT);
    digitalWrite(SWITCH_PIN,status?HIGH:LOW);
    switchStatus = status;
    Leds::motionStatus = status?Leds::MOTIONSTATUS::on:Leds::MOTIONSTATUS::off;
    logger.print(status?"SWT+":"SWT-");
}

void sensors_setup() {
    analogReadResolution(12);
    analogSetAttenuation(ADC_0db);
    logger.print("SNS+");
}

void serial_setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.flush();    
}

void setup() {
    motionWakeUp = (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0);
    if (!motionWakeUp) {
        ota_setup();
        wifi_setup();
        switch_setup(true);
    }
    else {
        serial_setup();
        wifi_setup();
        switch_setup(false);
    }
    leds.setup();
    ble_setup();
    bmi160_setup();
    sensors_setup();
    //logger.udpListen();
    xTaskCreate(manageSensors,"SNS",8192,NULL,1,&sensorsTask);
    xTaskCreate(manageComms,"COM",8192,NULL,1,&commTask);
    if (motionWakeUp) logger.println("MWUP");
    else logger.println("UP");
}

void loop() {
    check_incoming_commands();
    if (isGoToSleep) {
        //modem_sleep();
        switchOff();
        esp_deep_sleep();
        isGoToSleep = false;
    }
    if (isWakeUp) {
        //modem_awake();
        switchOn();
        isWakeUp = false;
    }
}