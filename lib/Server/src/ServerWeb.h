#ifndef Server_h
#define Server_h
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include "ArduinoNvs.h"

class webserver
{
public:
const char *ssid = "yourAP_test";
const char *password = "12345678";
String save_data;
int test_int;
void web_init();
void web_server();
};
extern webserver web;
#endif
