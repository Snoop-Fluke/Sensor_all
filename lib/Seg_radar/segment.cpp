#include "Seg_radar.h"
segment :: segment(uint8_t latchPin,uint8_t clockPin,uint8_t dataPin)
{
        pinMode(clockPin, OUTPUT);
        pinMode(dataPin, OUTPUT);
        pinMode(latchPin, OUTPUT);
        _latchPin = latchPin;
        _clockPin = clockPin;
        _dataPin = dataPin;
}
void segment :: LED_monitor(byte value, uint8_t speed_start, uint8_t limit_speed)
{
        if (value > speed_start )
        {
                if (value > limit_speed)
                {
                        x = 10;
                        m = 10;
                }
                else
                {
                        x = value % 10;
                        m = (value / 10) % 10;
                }
                digitalWrite(_latchPin, LOW);
                for (int i = 0; i < 16; i++) //หลักหน่วย
                {
                        digitalWrite(_dataPin, LOW);
                        if ( Array_1[x] & (0x001 << i) )
                        {
                                digitalWrite(_dataPin, HIGH);
                        }
                        digitalWrite(_clockPin, HIGH);
                        digitalWrite(_clockPin, LOW);
                }
                ///////////////////////////////////
                for (int i = 0; i < 16; i++) //หลักสิบ
                {
                        digitalWrite(_dataPin, LOW);
                        if ( Array_1[m] & (0x001 << i) )
                        {
                                digitalWrite(_dataPin, HIGH);
                        }

                        digitalWrite(_clockPin, HIGH);
                        digitalWrite(_clockPin, LOW);
                }
                digitalWrite(_latchPin, HIGH);
        }
}
