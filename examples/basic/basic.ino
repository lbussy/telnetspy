#include <Arduino.h>
#include <TelnetSpy.h>

#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#else // ESP32
#include <WiFi.h>
#include <ESPmDNS.h>
#endif

const char *ssid = "yourSSID";
const char *password = "yourPassword";

TelnetSpy SerialAndTelnet;

// #define SERIAL  Serial
#define SERIAL SerialAndTelnet

void waitForConnection()
{
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        SERIAL.print(".");
    }
    SERIAL.println(F(" Connected!"));
}

void waitForDisconnection()
{
    while (WiFi.status() == WL_CONNECTED)
    {
        delay(500);
        SERIAL.print(".");
    }
    SERIAL.println(F(" Disconnected!"));
}

void telnetConnected()
{
    SERIAL.println(F("Telnet connection established."));
}

void telnetDisconnected()
{
    SERIAL.println(F("Telnet connection closed."));
}

void disconnectClientWrapper()
{
    SERIAL.disconnectClient();
}

void setup()
{
    SERIAL.setWelcomeMsg(F("Welcome to the TelnetSpy.\r\n"));
    SERIAL.setCallbackOnConnect(telnetConnected);
    SERIAL.setCallbackOnDisconnect(telnetDisconnected);
    SERIAL.setFilter(char(1), F("\r\nNVT command: AO\r\n"), disconnectClientWrapper);
    SERIAL.begin(115200);
    delay(100); // Wait for serial port
    // SERIAL.setDebugOutput(false);
    SERIAL.print(F("Connecting to WiFi.."));
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    waitForConnection();

    SERIAL.print(F("IP address: "));
    SERIAL.println(WiFi.localIP());

    SERIAL.println(F("\r\nType 'C' for WiFi connect.\r\nType 'D' for WiFi disconnect.\r\nType 'R' for WiFi reconnect."));
    SERIAL.println(F("Type 'X' or Ctrl-A for closing telnet session.\r\n"));
    SERIAL.println(F("All other chars will be echoed. Play around...\r\n"));
    SERIAL.println(F("The following 'Special Commands' (telnet NVT protocol) are supported:"));
    SERIAL.println(F("  - Abort Output (AO) => closing telnet session."));
    SERIAL.println(F("  - Interrupt Process (IP) => restart the ESP.\r\n"));
}

void loop()
{
    SERIAL.handle();

    if (SERIAL.available() > 0)
    {
        char c = SERIAL.read();
        switch (c)
        {
        case '\r':
            SERIAL.println();
            break;
        case '\n':
            break;
        case 'C':
            SERIAL.print(F("\r\nConnecting.."));
            WiFi.begin(ssid, password);
            waitForConnection();
            break;
        case 'D':
            SERIAL.print(F("\r\nDisconnecting.."));
            WiFi.disconnect();
            waitForDisconnection();
            break;
        case 'R':
            SERIAL.print(F("\r\nReconnecting.."));
            WiFi.reconnect();
            waitForDisconnection();
            waitForConnection();
            break;
        case 'X':
            SERIAL.println(F("\r\nClosing telnet session.."));
            SERIAL.disconnectClient();
            break;
        default:
            SERIAL.print(c);
            break;
        }
    }
}
