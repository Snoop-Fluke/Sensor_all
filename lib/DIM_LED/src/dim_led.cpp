#include "dim_led.h"

int dim_led :: voltage_solar(int Pin_ana)
{
        float val;
        val = analogRead(Pin_ana);
        return (val*3.3/4095+0.24)*10;
}
unsigned int dim_led :: dimmer_LED(float val)
{
        float dimmer= 3.3-(val/10);
        dimmer >= 3 ? dimmer = 30 : dimmer < 3 && dimmer > 1.5 ? dimmer = 128 : dimmer < 1.5 ? dimmer = 220 : printf("LOW");
        // dimmer = (dimmer*192)/3.3;
        Serial.println("dim)led.h ");
        Serial.println(dimmer);
        return dimmer;
}
dim_led DIM_LED;
