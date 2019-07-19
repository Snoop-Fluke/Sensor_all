#include "Seg_radar.h"
#include "Murata.h"
#include "ByteConvert.h"
#include <EEPROM.h>
#include "blueconfig.h"
#include "Start_Murata.h"
#include "dim_led.h"
#define FREQ  5000
#define LEDCHANNEL  0
#define RESOLUTION  8

int buttonPushCounter = 0;   // counter for the number of button presses
int buttonState = 0;         // current state of the button
int lastButtonState = 0;

HardwareSerial MySerial(1);//Serial for Radar Module

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
segment seg(25,32,33);//set_pin_segment
void setup()
{
        Serial.begin(115200);//Serial baud rate
        MySerial.begin(9600, SERIAL_8N1, 35, 34);//set pin Tx,Rx for MySerial
        ledcSetup(LEDCHANNEL, FREQ, RESOLUTION);//set pwm channel
        ledcAttachPin(Oe_pin, LEDCHANNEL);//set pin pwm
        ledcWrite(LEDCHANNEL, 200);//set pwm channel
        blue_fn.blueinit();//call bluetooth begin
        seg.test_mode_up();
        seg.test_mode_down();
        // start_murata.initialize_radio();//ฟังก์ชั่นขารีเซ็ท LoRA
        xTaskCreate(Loop_BT_CF, "Loop_BT_CF", 2048, NULL, 1, NULL);//Task Bluetooth
        // xTaskCreate(Loop_LoRA_task, "Loop_LoRA_task", 2048, NULL, 2, NULL);//Task LoRA_Loop
        xTaskCreate(Loop_byte_Test, "Loop_byte_Test", 2048, NULL, 3, NULL);//Task Monitor
        xTaskCreate(Loop_LED_monitor, "Loop_LED_monitor", 2048, NULL, 5, NULL);//Task Monitor
}

void byte_Test()//Receive & Monitor Loop
{
        unsigned long currentMillis = millis();//current millis
        SOLAR_VOLT.val_volt = DIM_LED.voltage_solar(SOLAR_VOLT.SOLARVOLTAGE_PIN);//set brightness_fromSolar

        if (MySerial.available())
        {
                for (int i = 0; i < 9; i++)
                {
                        GET_DATA.data_array[i] = MySerial.read();
                }

                if (GET_DATA.data_array[0] == 170 && GET_DATA.data_array[4] > 4 && GET_DATA.data_array[4] != 0 && GET_DATA.data_array[3] == 1)
                {
                        if (currentMillis - DELAY.previousMillis >= DELAY.INTERVAL_DELAY_MONITOR) {
                                DELAY.previousMillis = currentMillis;//delay
                                GET_DATA.Sp_val = GET_DATA.data_array[4];//getdata from radar
                                ledcWrite(LEDCHANNEL, DIM_LED.dimmer_LED(SOLAR_VOLT.val_volt));//dim from solar
                                Serial.print("Speed :");
                                Serial.println(GET_DATA.data_array[4]);
                                buttonState = GET_DATA.Sp_val;

                                if (buttonState != lastButtonState) {

                                        if (GET_DATA.Sp_val > NVS.getInt("Sp_Lt"))
                                        {
                                                for (int i =0; i<4; i++)
                                                {
                                                        Serial.println("get_loop");
                                                        ledcWrite(LEDCHANNEL, 200);
                                                        //call Seg_radar.h.h
                                                        delay(500);
                                                        ledcWrite(LEDCHANNEL, 256);
                                                        delay(500);
                                                        //dim Led to 0

                                                }
                                        }

                                }
                                lastButtonState = buttonState;
                                seg.LED_monitor(GET_DATA.Sp_val,NVS.getInt("Sp_St"),NVS.getInt("Sp_Lt"));//set_startspeed&Alertspeed
                                Serial.print("dimmer  ");
                                Serial.println(DIM_LED.dimmer_LED(SOLAR_VOLT.val_volt));
                                Serial.print("Volt  ");
                                Serial.println(SOLAR_VOLT.val_volt);

                        }
                }
        }
        else
        {
                unsigned long currentMillis = millis();
                if (currentMillis - DELAY.previousMillis_2 >= 10000)//delayสำหรับset
                {
                        DELAY.previousMillis_2 = currentMillis;//delay
                        ledcWrite(LEDCHANNEL, 256);//dim Led to 0
                        GET_DATA.Sp_val = 0;//clear data
                }
        }
}
void Loop_byte_Test( void * parameter )//Run task
{
        while (1)
        {
                // Serial.println(val_volt);
                // Serial.println(dimmer_LED(val_volt));
                byte_Test();
                delay(2);
        }
        vTaskDelete( NULL );
}
void Loop_LoRA_task( void * parameter )//Run task
{
        while (1)
        {
                start_murata.Loop_LoRA(NVS.getInt("Am_car"));//set_Amount_car call Start_Murata.h
                start_murata.DOWNLINK();//call Start_Murata.h
                start_murata.Enable_status();//call Start_Murata.h
                delay(10);
        }
        vTaskDelete( NULL );
}
void Loop_LED_monitor( void * parameter )//Run task//Run task
{
        while (1)
        {
                seg.LED_monitor(GET_DATA.Sp_val,NVS.getInt("Sp_St"),NVS.getInt("Sp_Lt"));

                delay(10);
        }
        vTaskDelete( NULL );
}
void Loop_BT_CF( void * parameter )//Run task
{
        while (1)
        {
                blue_fn.blue_rx();//call blueconfig.h
                delay(1000);
                Serial.printf("val0\t%d val1\t%d val2\t%d\n",NVS.getInt("Sp_St"),NVS.getInt("Sp_Lt"),NVS.getInt("Am_car"));
                Serial.printf("%d\n",NVS.getInt("Am_car"));
        }
        vTaskDelete( NULL );
}
void loop()
{
}
