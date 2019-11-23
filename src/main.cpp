#include <Arduino.h>
#include "ThingsBoard.h"
#include <WiFi.h>
#define WIFI_NAME           "YR_WIFI"
#define WIFI_PASSWORD       "iloveYR_WIFI"
#define TOKEN               "owwRIL9PjSOGi4OobCzN"
#define THINGSBOARD_SERVER  "demo.thingsboard.io"

#define PH_SENSOR 34            //pH meter Analog output to Arduino Analog Input 0
#define RAINSERSOR 35
#define TRIGPIN 12
#define ECHOPIN 14
#define FLOW_SENSOR 5
#define EC_SENSOR 32
#define OFFSET -3.5           //deviation compensate

volatile int flow_frequency; // Measures flow sensor pulses

unsigned long currentTime;
unsigned long cloopTime;

void InitWiFi();
void reconnect();
void Task_PH_sensor(void *ignore);
void Task_EC_sensor(void *ignore);
void flow ();
WiFiClient espClient;
ThingsBoard tb(espClient);
int status = WL_IDLE_STATUS;

void setup() {
        Serial.begin(115200);
        pinMode(TRIGPIN,OUTPUT);

        pinMode(FLOW_SENSOR, INPUT);
        digitalWrite(FLOW_SENSOR, HIGH); // Optional Internal Pull-Up
        Serial.begin(9600);
        attachInterrupt(0, flow, RISING); // Setup Interrupt
        sei(); // Enable interrupts
        currentTime = millis();
        cloopTime = currentTime;

        // pinMode(ECHOPIN,OUTPUT);
        Serial.println("Wait_connect");
        InitWiFi();
        Serial.println(F("Connected to AP"));
        // xTaskCreate(Task_PH_sensor,"Task_PH_sensor",1024,NULL,5,NULL);
        // xTaskCreate(Task_EC_sensor,"Task_PH_sensor",1024,NULL,6,NULL);
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
        int ana = analogRead(PH_SENSOR);
        printf("analog_val\t%d\n",ana);
        for(int i=0; i<10; i++) //Get 10 sample value from the sensor for smooth the value
        {
                buf[i]=analogRead(PH_SENSOR);
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
        tb.sendTelemetryFloat("PH_SENSOR",phValue); //send_data
        return phValue;
}
int rain_sensor()
{
        int ana_read = 4095 - analogRead(RAINSERSOR);
        tb.sendTelemetryInt("RAIN_SENSOR",ana_read); //send_data
        return ana_read;
}

float ec_sensor()
{
        int sensor_val = analogRead(EC_SENSOR);
        Serial.println(sensor_val);
        float ec_val = sensor_val * (3.3/4095);
        tb.sendTelemetryInt("EC_SENSOR",sensor_val); //send_data
        return sensor_val;
}
uint8_t ultra_sensor()
{
        long duration;
        int distance;
        digitalWrite(TRIGPIN, LOW);
        delay(2);

        digitalWrite(TRIGPIN, HIGH);
        delay(10);
        digitalWrite(TRIGPIN, LOW);

        duration = pulseIn(ECHOPIN, HIGH);
        distance= (duration/2) / 29.1;
        tb.sendTelemetryInt("ULTRA_SENSOR",distance); //send_data
        return distance;
}
void flow () // Interrupt function
{
        flow_frequency++;
}
int flow_sensor()
{
        unsigned int l_hour; // Calculated litres/hour
        currentTime = millis();
        // Every second, calculate and print litres/hour
        if(currentTime >= (cloopTime + 1000))
        {
                cloopTime = currentTime; // Updates cloopTime
                // Pulse frequency (Hz) = 7.5Q, Q is flow rate in L/min.
                l_hour = (flow_frequency * 60 / 7.5); // (Pulse frequency x 60 min) / 7.5Q = flowrate in L/hour
                flow_frequency = 0; // Reset Counter
                Serial.print(l_hour, DEC); // Print litres/hour
                Serial.println(" L/hour");
        }
        return l_hour;
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
void loop()
{
        things_connect();
        Serial.print("PH_VAL : ");
        Serial.println(ph_sensor());
        delay(1000);
        Serial.print("RAIN_VAL : ");
        Serial.println(rain_sensor());
        delay(1000);
        Serial.print("ULTRA_VAL : ");
        Serial.println(ultra_sensor());
        delay(1000);
        Serial.print("EC_VAL1 : ");
        Serial.println(ec_sensor());
        delay(60000);
}
