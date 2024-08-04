#include "logging.h"
#include "ble.h"
#include <Arduino.h>

extern bool isModemSleepOn;

WiFiUDP Logger::udp;

void Logger::udpSendBroadcast(const char *message) {
    if (!WiFi.isConnected()) return;
    if (udp.beginPacket(WiFi.broadcastIP(),UDP_BROADCAST_PORT)) {
        udp.print(message);
        udp.endPacket();
    }
}

void Logger::udpListen() {
    if (!WiFi.isConnected()) return;
    udp.begin(UDP_LISTEN_PORT);
}

String Logger::udpReceive() {
    if (WiFi.isConnected()) {
        int bytes = udp.parsePacket();
        if (bytes>0) return udp.readString();
    }
    return String("");
}

void Logger::print(const char *message) {
    Serial.print(message);
    if (!isModemSleepOn) {
        udpSendBroadcast(message);
        ble_uart_send(message);
    }
}

void Logger::printf(const char *message, ...) {
    va_list argp;
    va_start(argp, message);

    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), message, argp);
    Logger::print(buffer);
}

void Logger::vprintf(const char *message, va_list argp) {
    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), message, argp);
    Logger::print(buffer);
}

void Logger::printfln(const char *message, ...) {
    va_list argp;
    va_start(argp, message);
    Logger::vprintf(message, argp);
    Logger::print("\n");
}

void Logger::println(const char *message) {
    Logger::print(message);
    Logger::print("\n");
}
