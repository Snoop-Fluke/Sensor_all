#include "Seg_radar.h"
#include "Murata.h"
#include "ByteConvert.h"
#include <EEPROM.h>
#include "blueconfig.h"
#include "Start_Murata.h"
#define FREQ  5000
#define LEDCHANNEL  0
#define RESOLUTION  8
HardwareSerial MySerial(1);

struct DELAY
{
        unsigned long INTERVAL_DELAY_MONITOR = 1000;
        unsigned long previousMillis =0;
        unsigned long previousMillis_2 =0;
} DELAY;
struct GET_DATA
{
        int Sp_val;
        int data_array[10];
} GET_DATA;
struct SOLAR_VOLT
{
        uint8_t SOLARVOLTAGE_PIN = 39;
        float val_volt;
} SOLAR_VOLT;
int *rx_data;
blueconfig blue_fn;
segment seg(25,32,33);//set_pin_segment
Start_Murata start_murata;
void setup()
{
        Serial.begin(115200);
        MySerial.begin(9600, SERIAL_8N1, 35, 34);
        ledcSetup(LEDCHANNEL, FREQ, RESOLUTION);
        ledcAttachPin(Oe_pin, LEDCHANNEL);
        ledcWrite(LEDCHANNEL, 256);
        blue_fn.blueinit();
        Serial.println(F("StartupLoRA"));
        // start_murata.initialize_radio();//ฟังก์ชั่นขารีเซ็ท LoRA
        xTaskCreate(Loop_BT_CF, "Loop_BT_CF", 2048, NULL, 1, NULL);
        // xTaskCreate(Loop_LoRA_task, "Loop_LoRA_task", 2048, NULL, 2, NULL);
        // xTaskCreate(Loop_byte_Test, "Loop_byte_Test", 4096, NULL, 3, NULL);
        // xTaskCreate(Loop_LED_monitor, "Loop_LED_monitor", 2048, NULL, 5, NULL);
}
int voltage_solar()
{
        float val;
        val = analogRead(SOLAR_VOLT.SOLARVOLTAGE_PIN);
        return (val*3.3/4095+0.24)*10;
}
uint dimmer_LED(float val)
{
        float dimmer= 3.3-(val/10);
        dimmer = (dimmer*192)/3.3;
        return dimmer;
}

void monitor_sleep()
{
        GET_DATA.Sp_val = 0;
        ledcWrite(LEDCHANNEL, 256);
}
void byte_Test()
{
        unsigned long currentMillis = millis();
        SOLAR_VOLT.val_volt = voltage_solar();

        if (MySerial.available())
        {
                for (int i = 0; i < 9; i++)
                {
                        GET_DATA.data_array[i] = MySerial.read();
                }

                if (GET_DATA.data_array[0] == 170 && GET_DATA.data_array[4] > 4 && GET_DATA.data_array[4] != 0 && GET_DATA.data_array[3] == 1)
                {
                        if (currentMillis - DELAY.previousMillis >= DELAY.INTERVAL_DELAY_MONITOR) {
                                DELAY.previousMillis = currentMillis;
                                GET_DATA.Sp_val = GET_DATA.data_array[4];
                                ledcWrite(LEDCHANNEL, dimmer_LED(SOLAR_VOLT.val_volt));
                                seg.LED_monitor(GET_DATA.Sp_val,NVS.getInt("Sp_St"),NVS.getInt("Sp_Lt"));//set_startspeed&Alertspeed
                                Serial.print("dimmer  ");
                                Serial.println(dimmer_LED(SOLAR_VOLT.val_volt));
                                Serial.print("Volt  ");
                                Serial.println(SOLAR_VOLT.val_volt);
                                Serial.print("Speed :");
                                Serial.println(GET_DATA.data_array[4]);
                        }
                }
        }
        else
        {
                unsigned long currentMillis = millis();
                if (currentMillis - DELAY.previousMillis_2 >= 10000) {
                        DELAY.previousMillis_2 = currentMillis;
                        ledcWrite(LEDCHANNEL, 256);
                        GET_DATA.Sp_val = 0;
                }
        }
}
// void Loop_byte_Test( void * parameter )
// {
//
//         while (1)
//         {
//                 // Serial.println(val_volt);
//                 // Serial.println(dimmer_LED(val_volt));
//                 byte_Test();
//                 delay(2);
//         }
//         vTaskDelete( NULL );
// }
// void Loop_LoRA_task( void * parameter )
// {
//         while (1)
//         {
//                 int* murata_cf = start_murata.DOWNLINK();
//                 start_murata.Loop_LoRA(NVS.getInt("Am_car"));//set_Amount_car
//                 start_murata.DOWNLINK();
//                 start_murata.Enable_status();
//                 delay(10);
//         }
//         vTaskDelete( NULL );
// }
// void Loop_LED_monitor( void * parameter )
// {
//         while (1)
//         {
//                 seg.LED_monitor(GET_DATA.Sp_val,NVS.getInt("Sp_St"),NVS.getInt("Sp_Lt"));
//                 delay(10);
//         }
//         vTaskDelete( NULL );
// }
void Loop_BT_CF( void * parameter )
{
        while (1)
        {
                blue_fn.blue_rx();
                delay(1000);
                Serial.printf("val0\t%d val1\t%d val2\t%d\n",NVS.getInt("Sp_St"),NVS.getInt("Sp_Lt"),NVS.getInt("Bluecf_3"));
        }
        vTaskDelete( NULL );
}
void loop()
{
}
