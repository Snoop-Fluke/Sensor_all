#include "blueconfig.h"
void blueconfig :: blueinit()
{
        NVS.begin();
        ESP_BT.begin("ESP32_TEST"); //Name of your Bluetooth Signal
        Serial.println("Bluetooth Device is Ready to Pair");

}
int* blueconfig :: blue_rx()
{

        if (ESP_BT.available())
        {
                for(int i =0; i < 4; i++)
                {
                        incoming[i] = ESP_BT.read();
                        Serial.print("Received:"); Serial.println(incoming[i],HEX);
                }
                ESP_BT.print("OK");
                NVS.setInt("Sp_St", incoming[0]);
                NVS.setInt("Sp_Lt", incoming[1]);
                NVS.setInt("Am_car", incoming[2]);
        }

        return incoming;
        delay(2);
}
blueconfig blue_fn;
