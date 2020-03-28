#include <Arduino.h>
#include "ThingsBoard.h"
#include <WiFi.h>
#include <ESP32Servo.h>
#include "DHT.h"
#include <pgmspace.h>
#include <EEPROM.h>
#include "Do_sensor.h"
#define WIFI_NAME           "snoop_fluke"
#define WIFI_PASSWORD       "fluke0902"
#define TOKEN               "owwRIL9PjSOGi4OobCzN"
#define THINGSBOARD_SERVER  "demo.thingsboard.io"
#define DHTPIN 2 //กำหนด pin DHT
#define DHTTYPE DHT22 //กำหนดชนิดของ DHT
#define PH_SENSOR 25         //pH meter Analog output to Arduino Analog Input 0
#define OFFSET -3.5           //deviation compensate
#define SERVO_TIME 1 //กำหนดเวลาServo ทำงาน
#define SENDDATA_TIME 1 //กำหนดเวลาส่งข้อมุล
int pos = 0;
#define SERVOPIN 5

int DoSensorPin = 35;
unsigned long previousMillis =0;
unsigned long previousMillis_2 =0;
unsigned long previousMillis_3 =0;

Servo myservo;
void InitWiFi();
void reconnect();
void Task_PH_sensor(void *ignore);
bool servo_loop();
void task_servo_loop(void *ignore);
WiFiClient espClient;
ThingsBoard tb(espClient);
int status = WL_IDLE_STATUS;
DHT dht;

void setup() {
        Serial.begin(115200);
        Serial.println("Wait_connect");
        InitWiFi();
        Serial.println(F("Connected to AP"));
        myservo.setPeriodHertz(50);// standard 50 hz servo
        myservo.attach(SERVOPIN, 1000, 2000);
        dht.setup(DHTPIN); // data pin 2
        pinMode(DoSensorPin,INPUT);
        _do_sensor.readDoCharacteristicValues();//read Characteristic Values calibrated from the EEPROM
        // xTaskCreate(Task_PH_sensor,"Task_PH_sensor",1024,NULL,5,NULL);
        // xTaskCreate(task_servo_loop,"task_servo_loop",2048,NULL,6,NULL);
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
void things_connect()
{
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis_3 >= 60000)//delayสำหรับset
        {
                previousMillis_3 = currentMillis;//delay
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

        }

}
// void task_servo_loop(void *ignore)
// {
//         while(1)
//         {
//                 Serial.println("test_task");
//                 bool servo_ = servo_loop();
//                 delay(10);
//         }
//         vTaskDelete( NULL );
// }
bool servo_loop()
{
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= 60000)//delayสำหรับset
        {
                previousMillis = currentMillis;//delay
                Serial.print("up_pos_");
                myservo.write(90); // tell servo to go to position in variable 'pos'
                delay(20);         // waits 15ms for the servo to reach the position
                tb.sendTelemetryFloat("Servo",1);
        }
        if (currentMillis - previousMillis_2 >= 60000)//delayสำหรับset
        {
                previousMillis_2 = currentMillis;//delay
                Serial.print("down_pos_");
                Serial.println(pos);
                myservo.write(0); // tell servo to go to position in variable 'pos'
                delay(20);
                tb.sendTelemetryFloat("Servo",0);
        }
        return true;
}
float *dht_loop()
{
        float *temp_hum = (float*) malloc(sizeof(float) * 2);
        *temp_hum = dht.getHumidity();
        *(temp_hum+1) = dht.getTemperature();
        tb.sendTelemetryFloat("HUM-",dht.getHumidity());
        tb.sendTelemetryFloat("TEMP-",dht.getTemperature());
        return (temp_hum);
}


void loop()
{

        float *arr = dht_loop();
        float hum = arr[0];
        float temp = arr[1];
        Serial.print("DO_sensor : ");
        Serial.println(_do_sensor.dosensor_loop(DoSensorPin));
        Serial.print("hum : ");
        Serial.print(hum);
        Serial.print("Temp : ");
        Serial.print(temp);
        Serial.print("PH_VAL : ");
        Serial.println(ph_sensor());
        things_connect(); //ส่งข้อมูลขึ้นเซริฟเวอร์
        delay(1000);

}
