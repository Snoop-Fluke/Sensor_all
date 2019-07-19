#ifndef Seg_radar_h
#define Seg_radar_h
#include <Arduino.h>

class segment
{
public:
segment(uint8_t latchPin,uint8_t clockPin,uint8_t dataPin);
// #define latchPin 25
// #define clockPin 32
// #define dataPin 33
  #define  Oe_pin 26
unsigned int Array_1[12] = {0xFFF0, 0x3C00, 0xF3CF, 0xFF0C, 0x3C3C, 0xCF3C, 0xCFFC, 0xFC00, 0xFFFC, 0xFF3C,0x000C,0x0000};
uint8_t m = 0;
uint8_t x = 0;

void test_mode_up();
void test_mode_down();
void LED_monitor(byte value, uint8_t speed_start, uint8_t limit_speed);
private:
uint8_t _latchPin;
uint8_t _clockPin;
uint8_t _dataPin;
};
#endif
