#include <Arduino.h>
#include "ThingsBoard.h"
#include <WiFi.h>
#define WIFI_NAME           "snoop_fluke"
#define WIFI_PASSWORD       "fluke0902"
#define TOKEN               "owwRIL9PjSOGi4OobCzN"
#define THINGSBOARD_SERVER  "demo.thingsboard.io"

#define SensorPin 34            //pH meter Analog output to Arduino Analog Input 0
#define RAINSERSOR 35
#define TRIGPIN 12
#define ECHOPIN 14
#define EC_SENSOR 32
#define OFFSET 0.00            //deviation compensate

void InitWiFi();
void reconnect();
void Task_PH_sensor(void *ignore);
void Task_EC_sensor(void *ignore);
WiFiClient espClient;
ThingsBoard tb(espClient);
int status = WL_IDLE_STATUS;

void setup() {
        Serial.begin(115200);
        pinMode(TRIGPIN,OUTPUT);
        // pinMode(ECHOPIN,OUTPUT);
        InitWiFi();
        Serial.println(F("Connected to AP"));
        xTaskCreate(Task_PH_sensor,"Task_PH_sensor",1024,NULL,5,NULL);
        xTaskCreate(Task_EC_sensor,"Task_PH_sensor",1024,NULL,6,NULL);
}
void InitWiFi()
{
        Serial.print(F("Connecting to AP ...  "));
        Serial.println(WIFI_NAME);
        WiFi.begin(WIFI_NAME, WIFI_PASSWORD);
        while (WiFi.status() != WL_CONNECTED) {
                delay(500);
                Serial.print(".");
        }
        Serial.print(F("Connected to AP _My address is  "));
        Serial.println(WiFi.localIP());
}
void reconnect()
{
        status = WiFi.status();
        if ( status != WL_CONNECTED) {
                WiFi.begin(WIFI_NAME, WIFI_PASSWORD);
                while (WiFi.status() != WL_CONNECTED) {
                        delay(500);
                        Serial.print(".");
                }
                Serial.println("Connected to AP");
        }
}

float ph_sensor()
{
        unsigned long int avgValue; //Store the average value of the sensor feedback

        int buf[10];          //buffer for read analog
        // int ana = analogRead(SensorPin);
        // printf("analog_val\t%d\n",ana);
        for(int i=0; i<10; i++) //Get 10 sample value from the sensor for smooth the value
        {
                buf[i]=analogRead(SensorPin);
                delay(10);
        }
        for(int i=0; i<9; i++) //sort the analog from small to large
        {
                for(int j=i+1; j<10; j++)
                {
                        if(buf[i]>buf[j])
                        {
                                int temp=buf[i];
                                buf[i]=buf[j];
                                buf[j]=temp;
                        }
                }
        }
        avgValue=0;
        for(int i=2; i<8; i++)              //take the average value of 6 center sample
                avgValue+=buf[i];
        float phValue=(float)avgValue*5.0/4095/6; //convert the analog into millivolt
        phValue=3.5*phValue+OFFSET;                //convert the millivolt into pH value
        // Serial.print("    pH:");
        // Serial.print(phValue,2);
        // Serial.println(" ");
        return phValue;
}
int rain_sensor()
{
        int ana_read = analogRead(RAINSERSOR);
        return ana_read;
}

float ec_sensor()
{
        int sensor_val = analogRead(EC_SENSOR);
        Serial.println(sensor_val);
        float ec_val = sensor_val * (5/4095);
        return ec_val;
}
uint8_t ultra_sensor()
{
        long duration;
        int distance;
        digitalWrite(TRIGPIN, LOW);
        delayMicroseconds(2);

        digitalWrite(TRIGPIN, HIGH);
        delayMicroseconds(10);
        digitalWrite(TRIGPIN, LOW);

        duration = pulseIn(ECHOPIN, HIGH);
        distance= (duration/2) / 29.1;
        return distance;
}
void things_connect()
{
  if (status != WL_CONNECTED) {
          Serial.println(F("Connecting to AP ..."));
          Serial.print(F("Attempting to connect to WPA SSID: "));
          reconnect();
  }

  if (!tb.connected()) {
          // Connect to the ThingsBoard
          Serial.print(F("Connecting to: "));
          Serial.print(THINGSBOARD_SERVER);
          Serial.print(F(" with token "));
          Serial.println(TOKEN);
          if (!tb.connect(THINGSBOARD_SERVER, TOKEN)) {
              Serial.println(F("Failed to connect"));
          }
  }
  // tb.sendTelemetryInt("temperature",ran_data); //send_data
}

void Task_PH_sensor(void *ignore)
{
  while(1)
  {
    Serial.println("test_task");
    delay(2000);
  }
}

void Task_EC_sensor(void *ignore)
{
  while(1)
  {
    Serial.println("test_task2");
    delay(2000);
  }
}

void loop() {
  // things_connect();
  // tb.sendTelemetryInt("temperature",100); //send_data
        // Serial.print("PH_VAL : ");
        // Serial.println(ph_sensor());
        // Serial.print("RAIN_VAL : ");
        // Serial.println(rain_sensor());
        // Serial.print("ULTRA_VAL : ");
        // Serial.println(ultra_sensor());
        // Serial.print("EC_VAL1 : ");
        // Serial.println(ec_sensor());
        delay(5000);

}
