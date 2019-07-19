#ifndef dim_led_h
#define dim_led_h
#include <Arduino.h>

class dim_led
{
public:
int voltage_solar(int Pin_ana);
unsigned int dimmer_LED(float val);
};
extern dim_led DIM_LED;
#endif
