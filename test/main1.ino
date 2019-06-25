#include "Seg_radar.h"
#include "Murata.h"
#include "ByteConvert.h"
#include <EEPROM.h>
#include "blueconfig.h"
#define FREQ  5000
#define LEDCHANNEL  0
#define RESOLUTION  8
HardwareSerial MySerial(1);
HardwareSerial loraSerial(2);
Murata myLora(loraSerial);
int *rx_data;
blueconfig blue_fn;
struct MURATA_LoRA
{
        unsigned long INTERVAL_RESET_COUNT = 7000;
        const char *devAddr = "6C:6D:00:21"; //รหัสdevice
        const char *nwkSKey = "43:49:56:49:43:4D:45:44:49:41:55:44:4F:4E:54:48";
        const char *appSKey = "4C:49:47:48:54:49:4E:47:50:4C:41:54:46:4F:52:4D";
        unsigned long INTERVAL_STATUS =60000*10;
        uint8_t COUNT_ROUND = 0;
        bool send_data_tx_LoRA = false;
} MURATA;
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
        Serial.begin(115200);
        MySerial.begin(9600, SERIAL_8N1, 35, 34);
        loraSerial.begin(9600, SERIAL_8N1, 16, 17);
        ledcSetup(LEDCHANNEL, FREQ, RESOLUTION);
        ledcAttachPin(Oe_pin, LEDCHANNEL);
        ledcWrite(LEDCHANNEL, 256);
        blue_fn.blueinit();
        Serial.println(F("StartupLoRA"));
        // initialize_radio();//ฟังก์ชั่นขารีเซ็ท LoRA
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
void initialize_radio()
{
        pinMode(4, OUTPUT);
        digitalWrite(4, LOW);
        delay(2000);
        digitalWrite(4, HIGH);
        delay(1000); //wait for startup message
        loraSerial.flush();
        Serial.print(F("Connection Murata:"));
        if (myLora.startat() == "\r\nOK\r\n")
        {
                Serial.println(F("OK"));
        }
        else
        {
                Serial.print(F("FAILED"));
                setup();
        }
        Serial.print(F("MuRata firmware version:"));
        Serial.println(myLora.sysver());
        Serial.print(F("Murata Class:"));
        Serial.println(myLora.sysclass());
        Serial.println(F("Trying to join ABP"));
        bool join_result = false;
        join_result = myLora.initABP(MURATA.devAddr, MURATA.appSKey, MURATA.nwkSKey);
        while (!join_result)
        {
                Serial.println(F("Unable to join. Are your keys correct, and do you have Network coverage?"));
                delay(5000); //delay a minute before retry
                join_result = myLora.initABP(MURATA.devAddr, MURATA.appSKey, MURATA.nwkSKey);
        }
        Serial.println(F("Successfully joined Network"));
        Serial.print(F("Murata DevAddr:"));
        Serial.println(myLora.info_devaddr());
        Serial.print(F("Murata appSKey:"));
        Serial.println(myLora.info_appskey());
        Serial.print(F("Murata nwkSKey:"));
        Serial.println(myLora.info_nwkskey());
        Serial.println(F("Successfully joined Network"));
        MURATA.send_data_tx_LoRA = myLora.sendTx("6702546802546A0000000013888E019200930194019510");
        MURATA.send_data_tx_LoRA = myLora.sendTx("6702546802546A0000000013888E019200930194019510");
        MURATA.send_data_tx_LoRA = myLora.sendTx("6702546802546A0000000013888E019200930194019510");
        // MURATA.send_data_tx_LoRA = myLora.sendTx("6702546802546A0000000013888E019200930194019510");
}
void UPLINK()
{
        MURATA.send_data_tx_LoRA = myLora.sendTx("6601");//คำสั่งส่ง 01 detect 00 nodetect
        if (MURATA.send_data_tx_LoRA == false)
        {
                initialize_radio();
                Serial.println(F("## Retry Join Network Success ##"));
        }
}
void Enable_status()
{
        unsigned long currentMillis = millis();
        if (currentMillis - DELAY.previousMillis >= MURATA.INTERVAL_STATUS) {
                DELAY.previousMillis = currentMillis;
                printf("RESET_COUNT_");
                for (int i=0; i<3; i++)
                {
                        MURATA.send_data_tx_LoRA = myLora.sendTx("6702546802546A0000000013888E019200930194019510");
                }
        }
}
void Loop_LoRA(uint8_t amount_car)
{
        unsigned long currentMillis = millis();
        if(MURATA.COUNT_ROUND % amount_car == 0 && MURATA.COUNT_ROUND !=0)
        {
                MURATA.COUNT_ROUND = 0;
                for(int i=0; i<3; i++)
                {
                        UPLINK();
                        delay(1000);
                }
                delay(10);
        }
        else
        {
                if (currentMillis - DELAY.previousMillis >= MURATA.INTERVAL_RESET_COUNT) {
                        DELAY.previousMillis = currentMillis;
                        MURATA.COUNT_ROUND = 0;
                }
        }
}

void DOWNLINK()
{
        String DataRx = myLora.getRx();
        uint8_t ptr = 0;
        char Data[11];
        DataRx.toCharArray(Data, 11);
        if (DataRx != "FAILED") {
                Serial.print(F("Received downlink : "));
                Serial.println(Data);
                while (ptr < (DataRx.length() - 1)) {
                        if (Data[ptr] == 'c' && Data[ptr + 1] == '1' )//config speed
                        {
                                ptr += 2;
                                uint8_t config_speed = htoi(Data[ptr]) * 16 + htoi(Data[ptr + 1]);
                                ptr += 2;
                                Serial.print(F("Config Speed : "));
                                Serial.println(config_speed);
                        }
                        else if (Data[ptr] == 'c' && Data[ptr + 1] == '2' )//config start detect
                        {
                                ptr += 2;
                                uint8_t st_detect = htoi(Data[ptr]) * 16 + htoi(Data[ptr + 1]);
                                ptr += 2;
                                Serial.print(F("Start Detect : "));
                                Serial.println(st_detect);
                        }
                        else
                        {
                                ptr = ptr + 2;
                        }
                }
                DataRx = "";
                for ( uint8_t i = 0; i < sizeof(Data); ++i ) //Clear_data
                        Data[i] = (char)0;
        }
}
void monitor_sleep()
{
        GET_DATA.Sp_val = 0;
        ledcWrite(LEDCHANNEL, 256);
}
void byte_Test(uint8_t pwm_set)
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
                                seg.LED_monitor(GET_DATA.Sp_val,*(rx_data+0),*(rx_data+1));
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
//                 byte_Test(5);
//                 delay(2);
//         }
//         vTaskDelete( NULL );
// }
// void Loop_LoRA_task( void * parameter )
// {
//         while (1)
//         {
//
//                 Loop_LoRA(*(rx_data+2));
//                 DOWNLINK();
//                 Enable_status();
//                 delay(10);
//         }
//         vTaskDelete( NULL );
// }
// void Loop_LED_monitor( void * parameter )
// {
//         while (1)
//         {
//                 seg.LED_monitor(GET_DATA.Sp_val,*(rx_data+0),*(rx_data+1));
//                 delay(10);
//         }
//         vTaskDelete( NULL );
// }
void Loop_BT_CF( void * parameter )
{
        while (1)
        {
                rx_data = blue_fn.blue_rx();
                delay(1000);
                Serial.printf("val0\t%d val1\t%d val2\t%d\n",*(rx_data+0),*(rx_data+1),*(rx_data+1));
        }
        vTaskDelete( NULL );
}
void loop()
{
}
