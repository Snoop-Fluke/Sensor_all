#include "blueconfig.h"
void blueconfig :: blueinit()
{
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
        }
        return incoming;
        delay(2);
}
