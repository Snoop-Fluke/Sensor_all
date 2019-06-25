#include "Start_Murata.h"
HardwareSerial loraSerial(2);
Murata myLora(loraSerial);
Start_Murata :: Start_Murata()
{
        loraSerial.begin(9600, SERIAL_8N1, 16, 17);
}
void Start_Murata :: initialize_radio()
{
        // loraSerial.begin(9600, SERIAL_8N1, 16, 17);
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
        MURATA.send_data_tx_LoRA = myLora.sendTx("6702546802546A0000000013888E019200930194019510");
        MURATA.send_data_tx_LoRA = myLora.sendTx("6702546802546A0000000013888E019200930194019510");
}

void Start_Murata :: UPLINK()
{
        MURATA.send_data_tx_LoRA = myLora.sendTx("6601");//คำสั่งส่ง 01 detect 00 nodetect
        if (MURATA.send_data_tx_LoRA == false)
        {
                initialize_radio();
                Serial.println(F("## Retry Join Network Success ##"));
        }
}
void Start_Murata :: Enable_status()
{
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= MURATA.INTERVAL_STATUS) {
                previousMillis = currentMillis;
                printf("RESET_COUNT_");
                for (int i=0; i<3; i++)
                {
                        MURATA.send_data_tx_LoRA = myLora.sendTx("6702546802546A0000000013888E019200930194019510");
                }
        }
}
void Start_Murata :: Loop_LoRA(uint8_t amount_car)
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
                if (currentMillis - previousMillis >= MURATA.INTERVAL_RESET_COUNT) {
                        previousMillis = currentMillis;
                        MURATA.COUNT_ROUND = 0;
                }
        }
}

void Start_Murata :: DOWNLINK()
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
