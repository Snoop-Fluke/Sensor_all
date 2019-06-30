#ifndef blueconfig_h
#define blueconfig_h
#include <Arduino.h>
#include "BluetoothSerial.h"
#include "ArduinoNvs.h"
class blueconfig
{
public:
BluetoothSerial ESP_BT;
void blueinit();
int* blue_rx();
int incoming[4] = {10, 20, 15, 1};
};
#endif
