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

#if __has_include("./secrets.h")
#include "secrets.h" // Include for AP_NAME and PASSWD below
const char *ssid = AP_NAME;
const char *password = PASSWRD;
const char *hostname = HOSTNAME;
const int baud = BAUD;
#else
const char *ssid = "my_ap";
const char *password = "passsword";
const char *hostname = "telnethost";
const int baud = 115200;
#endif

TelnetSpy SerialAndTelnet;

// #define SERIAL  Serial
#undef SERIAL
#define SERIAL SerialAndTelnet

void waitForConnection()
{
    SERIAL.print(F("Connecting to WiFi.."));
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        SERIAL.print(".");
    }
    SERIAL.println();
    SERIAL.println(F("Connected!"));
}

void waitForDisconnection()
{
    SERIAL.print(F("Disconnecting from WiFi.."));
    while (WiFi.status() == WL_CONNECTED)
    {
        delay(500);
        SERIAL.print(".");
    }
    SERIAL.println();
    SERIAL.println(F("Disconnected!"));
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
    SERIAL.begin(baud);
    delay(100); // Wait for serial port
    // SERIAL.setDebugOutput(false);
    WiFi.disconnect(true);
#ifdef ESP8266
    WiFi.hostname(hostname);
#else
    WiFi.setHostname(hostname);
#endif
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    waitForConnection();

#ifdef ESP8266
    MDNS.begin(hostname);
#else // ESP32
    MDNS.begin(hostname);
#endif
    MDNS.addService("telnet", "tcp", 23);
    SERIAL.print(F("MDNS started, hostname: "));
    SERIAL.print(hostname);
    SERIAL.println(".local");

    SERIAL.print(F("IP address: "));
    SERIAL.println(WiFi.localIP());

    SERIAL.println(F("\r\nType 'C' for WiFi.\r\nType 'D' for WiFi disconnect.\r\nType 'R' for WiFi reconnect.\r\nType 'T' for Telnet toggle."));
    SERIAL.println(F("Type 'X' or Ctrl-A for closing telnet session.\r\n"));
    SERIAL.println(F("All other chars will be echoed. Play around...\r\n"));
    SERIAL.println(F("The following 'Special Commands' (telnet NVT protocol) are supported:"));
    SERIAL.println(F("  - Abort Output (AO) => closing telnet session."));
    SERIAL.println(F("  - Interrupt Process (IP) => restart the ESP.\r\n"));
}

void loop()
{
    MDNS.update();
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
        case 'T':
            if (SERIAL.enabled())
            {
                SERIAL.toggle(false);
                SERIAL.println(F("\r\nTelnet disabled."));
            }
            else
            {
                SERIAL.toggle(true);
                SERIAL.println(F("\r\nTelnet enabled."));
            }
            break;
        default:
            SERIAL.print(c);
            break;
        }
    }
}
