#include "Murata.h"
#include "ByteConvert.h"
#include <EEPROM.h>
#include "Start_Murata.h"
void setup()
{
        start_murata.initialize_radio();//ฟังก์ชั่นขารีเซ็ท LoRA
        xTaskCreate(Loop_LoRA_task, "Loop_LoRA_task", 2048, NULL, 2, NULL);
}
void Loop_LoRA_task( void * parameter )
{
        while (1)
        {
                start_murata.DOWNLINK();

                start_murata.Loop_LoRA(NVS.getInt("Am_car"));//set_Amount_car
                start_murata.Enable_status();
                delay(10);
        }
        vTaskDelete( NULL );
}
void loop()
{
}
