#include <Arduino.h>
#include "Seg_radar.h"
#include "Murata.h"
#include "ByteConvert.h"
#include <EEPROM.h>
#include "blueconfig.h"
#include "Start_Murata.h"
#include "dim_led.h"
#define FREQ 5000
#define LEDCHANNEL 0
#define RESOLUTION 8
#define OVERFLOW 99
#define NORMAL 11
#define SOLAR_LIMIT 50
#define TIME_TO_SEND 60000
#define LATCH_PIN 25
#define CLOCK_PIN 32
#define DATA_PIN 33
#define SPEED_OVERFLOW 90
#define SPEED_START 15 //set_speed_start
#define SPEED_LIMIT 30 //set_speed_limit
#define AMOUNT_CAR 30
#define BLINK_ALERT 100
#define MONITOR_TIME 10 //10 sec
#define DETECT_TIME 11  //c6 1.1 sec
#define TX_PIN 35
#define RX_PIN 34
int buttonPushCounter = 0; // counter for the number of button presses
int buttonState = 0;       // current state of the button
int lastButtonState = 0;
uint8_t Count_oop = 0;
uint32_t Count_car = 0;
HardwareSerial MySerial(1); //Serial for Radar Module

struct DELAY
{
        unsigned long INTERVAL_DELAY_MONITOR = 100;
        unsigned long previousMillis = 0;
        unsigned long previousMillis_2 = 0;
        unsigned long previousMillis_3 = 0;
} DELAY;
struct GET_DATA
{
        int Sp_val;
        int data_array[10];
} GET_DATA;
struct SOLAR_VOLT
{
        float val_volt;
        float BATT;
} SOLAR_VOLT;
void Loop_LED_monitor(void *parameter);
void Loop_BT_CF(void *parameter);
void Loop_LoRA_task(void *parameter);
void Loop_byte_Test(void *parameter); //Run task
segment seg(LATCH_PIN, CLOCK_PIN, DATA_PIN); //set_pin_segment
void setup()
{
        Serial.begin(115200); //Serial baud rate
        // Serial.println("Speed_detec_NonLoRA");
        Serial.println("Speed_detec_LoRA");
        MySerial.begin(9600, SERIAL_8N1, TX_PIN, RX_PIN); //set pin Tx,Rx for MySerial
        ledcSetup(LEDCHANNEL, FREQ, RESOLUTION);          //set pwm channel
        ledcAttachPin(Oe_pin, LEDCHANNEL);                //set pin pwm
        ledcWrite(LEDCHANNEL, 200);                       //set pwm channel
        NVS.begin();
        delay(3000); //start Nvs_flash
        delay(500);
        // xTaskCreate(Loop_BT_CF, "Loop_BT_CF", 2048, NULL, 6, NULL);             //Task Bluetooth
        xTaskCreate(Loop_LoRA_task, "Loop_LoRA_task", 2048, NULL, 5, NULL);     //Task LoRA_Loop
        xTaskCreate(Loop_byte_Test, "Loop_byte_Test", 2048, NULL, 3, NULL);     //Task Monitor
        xTaskCreate(Loop_LED_monitor, "Loop_LED_monitor", 2048, NULL, 7, NULL); //Task Monitor
}
void byte_Test() //Receive & Monitor Loop
{
        unsigned long currentMillis = millis(); //current millis
        float solar = DIM_LED.voltage_solar();
        int batt = DIM_LED.voltage_batt();
        SOLAR_VOLT.val_volt = solar; //read_solar_analog
        SOLAR_VOLT.BATT = batt;
        if (MySerial.available())
        {
                for (int i = 0; i < 9; i++)
                {
                        GET_DATA.data_array[i] = MySerial.read(); //read_data_formRadar
                }
                if (GET_DATA.data_array[0] == 170 && GET_DATA.data_array[4] > 4 && GET_DATA.data_array[4] != 0 && GET_DATA.data_array[3] == 1) //check_data_input
                {
                        if (currentMillis - DELAY.previousMillis >= DELAY.INTERVAL_DELAY_MONITOR * NVS.getInt("detect")) //delay
                        {
                                DELAY.previousMillis = currentMillis;     //delay
                                GET_DATA.Sp_val = GET_DATA.data_array[4]; //getdata from radar
                                buttonState = GET_DATA.Sp_val;
                                if (GET_DATA.Sp_val > NVS.getInt("Sp_St"))
                                {
                                        ledcWrite(LEDCHANNEL, DIM_LED.dimmer_LED(SOLAR_VOLT.val_volt)); //dim from solar
                                }
                                if (buttonState != lastButtonState)
                                {
                                        Count_oop++; //add_for_sent_LoRA
                                        Count_car++;
                                        blue_fn.ESP_BT.print("Count_oop    ");
                                        blue_fn.ESP_BT.println(Count_oop);
                                }
                                lastButtonState = buttonState;
                                Serial.print("dimmer  ");
                                Serial.println(DIM_LED.dimmer_LED(SOLAR_VOLT.val_volt));
                                Serial.print("Volt  ");
                                Serial.println(SOLAR_VOLT.val_volt);
                                Serial.print("BATT  ");
                                Serial.println(SOLAR_VOLT.BATT);
                        }
                }
        }
        else
        {
                unsigned long currentMillis = millis();
                if (currentMillis - DELAY.previousMillis_2 >= NVS.getInt("monitor") * 1000) //delay_for_monitor
                {
                        DELAY.previousMillis_2 = currentMillis; //delay
                        ledcWrite(LEDCHANNEL, 256);             //dim Led to 0
                        GET_DATA.Sp_val = 0;                    //clear data
                }
        }
}
void switch_set(int set_data, String str, int value)
{
        switch (set_data)
        {
        case 0:
                NVS.setInt(str, value);
                break;
        }
        // Serial.printf("val. %d\n",NVS.getInt(str));
}
void Loop_byte_Test(void *parameter) //Run task
{
        while (1)
        {
                byte_Test(); //byte_Test_Loop
                delay(10);
        }
        vTaskDelete(NULL);
}
void Loop_LoRA_task(void *parameter) //Run task
{
        start_murata.initialize_radio(); //ฟังก์ชั่นขารีเซ็ท LoRA
        Serial.println("Start_LoRA");
        while (1)
        {
                if (SOLAR_VOLT.val_volt < 50)
                {
                        if (Count_oop >= NVS.getInt("Am_car"))
                        {

                                Serial.printf("Count_oop\t%d\tAm_car\t%lli\n", Count_oop, NVS.getInt("Am_car"));
                                Count_oop = 0; //reset_Count
                                Serial.println("send_loop");
                                start_murata.Loop_LoRA(); //set_Amount_car call Start_Murata.h
                        }
                        else
                        {
                                unsigned int num_time = 60000;
                                unsigned long delay_of_setzero = (num_time * 2); //setlongtime
                                unsigned long currentMillis = millis();
                                if (currentMillis - DELAY.previousMillis_3 >= delay_of_setzero) //delay_for_reset_count
                                {
                                        DELAY.previousMillis_3 = currentMillis;
                                        Serial.println("Set_Zero");
                                        Count_oop = 0; //reset_count
                                        Serial.printf("Count_oop\t%d\tAm_car\t%lli\n", Count_oop, NVS.getInt("Am_car"));
                                }
                        }
                }
                start_murata.Enable_status(SOLAR_VOLT.BATT, SOLAR_VOLT.val_volt, Count_car); //call Start_Murata.h
                start_murata.DOWNLINK();                                                     //call Start_Murata.h
                delay(10);
        }
        vTaskDelete(NULL);
}
void Loop_LED_monitor(void *parameter) //Run task//Run task
{
        while (1)
        {
                if (GET_DATA.Sp_val > OVERFLOW)
                {
                        seg.LED_monitor(GET_DATA.Sp_val, OVERFLOW);
                }
                else
                {
                        seg.LED_monitor(GET_DATA.Sp_val, NORMAL);
                        while (GET_DATA.Sp_val > NVS.getInt("Sp_Lt")) //blink_if_limit_speed
                        {
                                seg.LED_monitor(GET_DATA.Sp_val, NORMAL);
                                ledcWrite(LEDCHANNEL, DIM_LED.dimmer_LED(SOLAR_VOLT.val_volt)); //monitor_segment                                     //DIM_ZERO
                                delay(NVS.getInt("blink"));                                     //set_form_NVS
                                ledcWrite(LEDCHANNEL, 256);
                                delay(NVS.getInt("blink")); //set_form_NVS
                                Serial.printf("blink_delay%d\n", NVS.getInt("blink"));
                                //dim Led to 0
                        }

                        delay(10);
                }
        }
        vTaskDelete(NULL);
}
void Loop_BT_CF(void *parameter) //Run task
{

        while (1)
        {

                blue_fn.Serial_read(); //call blueconfig.h
                // float solar = DIM_LED.voltage_solar();
                // float batt = DIM_LED.voltage_batt();
                // solar = (solar / 10);
                switch_set(NVS.getInt("Sp_St"), "Sp_St", SPEED_START);      //c1
                switch_set(NVS.getInt("Sp_Lt"), "Sp_Lt", SPEED_LIMIT);      //c2
                switch_set(NVS.getInt("Am_car"), "Am_car", AMOUNT_CAR);     //c3
                switch_set(NVS.getInt("blink"), "blink", BLINK_ALERT);      //c4
                switch_set(NVS.getInt("monitor"), "monitor", MONITOR_TIME); //c5/1000 1 sec
                switch_set(NVS.getInt("detect"), "detect", DETECT_TIME);    //c6 1.1 sec
                Serial.print("c1 Sp_St    ");
                Serial.printf("%d\n", NVS.getInt("Sp_St"));
                Serial.print("c2 Sp_Lt    ");
                Serial.printf("%d\n", NVS.getInt("Sp_Lt"));
                Serial.print("c3 Am_car    ");
                Serial.printf("%d\n", NVS.getInt("Am_car"));
                Serial.print("c4 blink    ");
                Serial.printf("%d\n", NVS.getInt("blink"));
                Serial.print("c5 monitor    ");
                Serial.printf("%d\n", NVS.getInt("monitor"));
                Serial.print("c6 detect    ");
                Serial.printf("%d\n", NVS.getInt("detect"));


                delay(1500);
                delay(20);
        }
        vTaskDelete(NULL);
}
void loop()
{
        delay(1000);
}
