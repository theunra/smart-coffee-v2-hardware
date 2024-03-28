#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "ArduinoJson.h"
#include "ADS1X15.h"
#include "Adafruit_SHT31.h"
#include "memory.h"

#define DEBUG 0

#define I2C_ADDR_ADS1    0x48 // GND
#define I2C_ADDR_ADS2    0x49 // VCC
#define I2C_ADDR_SHT21   0x44

#define IO_INTERNAL_LED  2
#define IO_PWM1          25
#define IO_PWM2          26
#define LEDC_CHAN_PWM1   0
#define LEDC_CHAN_PWM2   1
#define LEDC_FREQ        5000
#define LEDC_RESO        8

#define ADS_GAIN          0 //6.144V
#define ADS_VOLTAGE       6.144
#define ADS_ADC           16
#define ADS1_CHAN_MQ137   0
#define ADS1_CHAN_MQ3     1
#define ADS1_CHAN_MQ138   2
#define ADS1_CHAN_MQ135   3
#define ADS2_CHAN_MQ2     0
#define ADS2_CHAN_MQ136   1
#define ADS2_CHAN_TGS822  2
#define ADS2_CHAN_TGS2620 3

#define pcSerial Serial

namespace ROASTING_LEVEL {
    enum {CHARGE, LIGHT, MEDIUM, DARK};
}

ADS1115 ads1(I2C_ADDR_ADS1);
ADS1115 ads2(I2C_ADDR_ADS2);

Adafruit_SHT31 sht31 = Adafruit_SHT31(); // SHT31 sensor object

uint16_t adc_mq135   = 0;
uint16_t adc_mq136   = 0;
uint16_t adc_mq137   = 0;
uint16_t adc_mq138   = 0;
uint16_t adc_mq2     = 0;
uint16_t adc_mq3     = 0;
uint16_t adc_tgs822  = 0;
uint16_t adc_tgs2620 = 0;

void init_hardwares();
void init_check_sensors();

void adsCallback();

void fail();

void setup()
{
    init_hardwares();      // GPIO and such
    init_check_sensors();  // Initial check for sensors
}

unsigned long last_ms = 0;

void loop() 
{
    unsigned long ms = millis();
    adsCallback();

    unsigned long done_ms;
    
    do {
        done_ms = millis();
    } while((done_ms - ms) < 35);

    // pcSerial.print("delay : ");
    // pcSerial.println(done_ms - ms);
}

void init_hardwares()
{
    ledcSetup(LEDC_CHAN_PWM1, LEDC_FREQ, LEDC_RESO);
    ledcSetup(LEDC_CHAN_PWM2, LEDC_FREQ, LEDC_RESO);

    ledcAttachPin(IO_PWM1, LEDC_CHAN_PWM1);
    ledcAttachPin(IO_PWM2, LEDC_CHAN_PWM2);

    ledcWrite(LEDC_CHAN_PWM1, 210);
    ledcWrite(LEDC_CHAN_PWM2, 210);

    Wire.begin();

    pcSerial.begin(115200);

    pcSerial.println("ok");
}

void init_check_sensors()
{
    if (!sht31.begin(I2C_ADDR_SHT21))
    {
        pcSerial.println("{\"error\" : \"SHT31 ERROR!\"}");
        fail();
    }

    if (sht31.isHeaterEnabled()) 
        pcSerial.println("{\"info\" : \"SHT31 Heater Enabled!\"}");
    else 
        pcSerial.println("{\"info\" : \"SHT31 Heater Disabled!\"}");

    if(!ads1.isConnected()){
        pcSerial.println("{\"error\" : \"ADS 1 ERROR!\"}");
        fail();
    }

    if(!ads2.isConnected()){
        pcSerial.println("{\"error\" : \"ADS 2 ERROR!\"}");
        fail();
    }

    ads1.setGain(ADS_GAIN);
    ads2.setGain(ADS_GAIN);
    ads1.setDataRate(7);
    ads2.setDataRate(7);
}

char buffer[100];

void adsCallback()
{
    // read gas sensors adc
    adc_mq135   = ads1.readADC(ADS1_CHAN_MQ135);
    adc_mq136   = ads2.readADC(ADS2_CHAN_MQ136);
    adc_mq137   = ads1.readADC(ADS1_CHAN_MQ137);
    adc_mq138   = ads1.readADC(ADS1_CHAN_MQ138);
    adc_mq2     = ads2.readADC(ADS2_CHAN_MQ2);
    adc_mq3     = ads1.readADC(ADS1_CHAN_MQ3);
    adc_tgs822  = ads2.readADC(ADS2_CHAN_TGS822);
    adc_tgs2620 = ads2.readADC(ADS2_CHAN_TGS2620);

    // SHT31 sensor readings
    float t = -99;
    float h = -99;
    sht31.readBoth(&t, &h);
    // float t = sht31.readTemperature();
    // float h = sht31.readHumidity();

    if (isnan(t)){t = -99;}
    if (isnan(h)){h = -99;}

    // send data to m6
    #define SENSOR_DATA_SIZE 10

    const int sensor_json_cap = JSON_OBJECT_SIZE(SENSOR_DATA_SIZE);
    StaticJsonDocument<sensor_json_cap> sensor_json;

    sensor_json["adc_mq135"]   = adc_mq135;
    sensor_json["adc_mq136"]   = adc_mq136;
    sensor_json["adc_mq137"]   = adc_mq137;
    sensor_json["adc_mq138"]   = adc_mq138;
    sensor_json["adc_mq2"]     = adc_mq2;
    sensor_json["adc_mq3"]     = adc_mq3;
    sensor_json["adc_tgs822"]  = adc_tgs822;
    sensor_json["adc_tgs2620"] = adc_tgs2620;
    sensor_json["temp"]        = t;
    sensor_json["humidity"]    = h;

    serializeJson(sensor_json, pcSerial);
    // memcpy(buffer, &adc_mq135, sizeof(uint16_t));
    // pcSerial.print(adc_mq135);
    pcSerial.print("\n");
}

void fail()
{
    pcSerial.println("{\"error\" : \"fail\"}");
    while(1){}
}