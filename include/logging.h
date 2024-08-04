#pragma once

#include <stdarg.h>
#include <WiFi.h>

#define UDP_LISTEN_PORT 8082
#define UDP_BROADCAST_PORT 8081

class Logger {
    private:
        static WiFiUDP udp;
        static void udpSendBroadcast(const char *);
    public:
        static void udpListen();
        static String udpReceive(); 
        static void print(const char *message);
        static void printf(const char *message, ...);
        static void vprintf(const char *message, va_list argp);
        static void printfln(const char *message, ...);
        static void println(const char* message);
};