#ifndef Start_Murata_h
#define Start_Murata_h
#include <Arduino.h>
#include "Murata.h"
#include "ByteConvert.h"
#include "ArduinoNvs.h"

class Start_Murata
{

public:
Start_Murata();
struct MURATA_LoRA
{
        unsigned long INTERVAL_RESET_COUNT = 7000;
        const char *devAddr = "6C:6D:00:21";         //รหัสdevice
        const char *nwkSKey = "43:49:56:49:43:4D:45:44:49:41:55:44:4F:4E:54:48";
        const char *appSKey = "4C:49:47:48:54:49:4E:47:50:4C:41:54:46:4F:52:4D";
        unsigned long INTERVAL_STATUS =60000*10;
        uint8_t COUNT_ROUND = 0;
        bool send_data_tx_LoRA = false;
        int config_speed[4] = {50,10,10,10};
} MURATA;
unsigned long previousMillis =0;
void initialize_radio();
void UPLINK();
void Enable_status();
void Loop_LoRA(uint8_t amount_car);
int* DOWNLINK();
};
extern Start_Murata start_murata;
#endif
