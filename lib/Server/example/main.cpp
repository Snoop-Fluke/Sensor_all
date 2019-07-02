#include <Arduino.h>
#include "Server.h"
#include "Seg_radar.h"

void setup() {

        Serial.begin(115200);
        web.web_init();
}

void loop() {

        web.web_server();
}
