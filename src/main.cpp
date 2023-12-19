#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Preferences.h>
#include "TaskScheduler.h"
#include "ArduinoJson.h"
#include "ADS1X15.h"
#include "Adafruit_SHT31.h"
#include "MQUnifiedsensor.h"
#include "TGS2620.h"
#include "Nextion.h"

#define DEBUG 0

#define PREF_NAMESPACE_ENOSE_R0 "enose_r0"

#define ADC16TO8 0.0038909912109375

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

#define NEX_BUTTON_RESET_PID 0
#define NEX_BUTTON_RESET_CID 5
#define NEX_BUTTON_RESET_NAME "b2"

#define NEX_WAVE1_PID 0
#define NEX_WAVE1_CID 1
#define NEX_WAVE1_NAME "s0"
#define NEX_WAVE2_PID 0
#define NEX_WAVE2_CID 2
#define NEX_WAVE2_NAME "s1"

#define NEX_WAVE1_CHAN_MQ135   0
#define NEX_WAVE1_CHAN_MQ136   1
#define NEX_WAVE1_CHAN_MQ137   2
#define NEX_WAVE1_CHAN_MQ138   3
#define NEX_WAVE2_CHAN_MQ2     0
#define NEX_WAVE2_CHAN_MQ3     1
#define NEX_WAVE2_CHAN_TGS822  2
#define NEX_WAVE2_CHAN_TGS2620 3

#define TASK_INTERVAL_MS_ADS 1
#define TASK_INTERVAL_MS_NEX 10

#define USE_NEXTION_GRAPH 0
#define IS_CALIBRATING_GAS_SENSOR 0
#define USE_PPM 1

bool is_run = true;

Preferences pref;

// TaskHandle_t t0H;
Scheduler sch;

// NexButton button_reset = NexButton(NEX_BUTTON_RESET_PID, NEX_BUTTON_RESET_CID, NEX_BUTTON_RESET_NAME);

#if USE_NEXTION_GRAPH
NexWaveform wave1 = NexWaveform(NEX_WAVE1_PID, NEX_WAVE1_CID, NEX_WAVE1_NAME);
NexWaveform wave2 = NexWaveform(NEX_WAVE2_PID, NEX_WAVE2_CID, NEX_WAVE2_NAME);
#endif

// NexTouch *nex_listen_list[] = {
//     &button_reset,
//     NULL
// };

// void buttonResetCb(void*);

ADS1115 ads1(I2C_ADDR_ADS1);
ADS1115 ads2(I2C_ADDR_ADS2);

Adafruit_SHT31 sht31 = Adafruit_SHT31(); // SHT31 sensor object

MQUnifiedsensor mq135 ("ESP32", ADS_VOLTAGE, ADS_ADC, 0, "MQ-135");
MQUnifiedsensor mq136 ("ESP32", ADS_VOLTAGE, ADS_ADC, 0, "MQ-136");
MQUnifiedsensor mq137 ("ESP32", ADS_VOLTAGE, ADS_ADC, 0, "MQ-137");
MQUnifiedsensor mq138 ("ESP32", ADS_VOLTAGE, ADS_ADC, 0, "MQ-138");
MQUnifiedsensor mq2   ("ESP32", ADS_VOLTAGE, ADS_ADC, 0, "MQ-2");
MQUnifiedsensor mq3   ("ESP32", ADS_VOLTAGE, ADS_ADC, 0, "MQ-3");
TGS2620 tgs2620;
TGS2620 tgs822; // rumus tgs2620 sm tgs822 sama, beda a b

uint16_t adc_mq135   = 0;
uint16_t adc_mq136   = 0;
uint16_t adc_mq137   = 0;
uint16_t adc_mq138   = 0;
uint16_t adc_mq2     = 0;
uint16_t adc_mq3     = 0;
uint16_t adc_tgs822  = 0;
uint16_t adc_tgs2620 = 0;

void Core0Task(void*);

void fail();

void init_hardwares();
void init_check_sensors();
void init_tasks();

void adsCallback();
void nexCallback();

Task task_ads(TASK_INTERVAL_MS_ADS, TASK_FOREVER, &adsCallback);
Task task_nex(TASK_INTERVAL_MS_NEX, TASK_FOREVER, &nexCallback);


void setup()
{
    init_hardwares();      // GPIO and such
    init_check_sensors();  // Initial check for sensors
    init_tasks();
}

void loop() 
{
    sch.execute(); // Execute scheduled tasks
}


void Core0Task(void* vParam)
{
    String message = "";
    for(;;)
    {
        if(Serial.available() > 0){
            while(Serial.available() > 0)
            {
                message += Serial.read();
            }
            Serial.println(message);
            message = "";
        }
        sleep(1000);
    }
}

void fail()
{
    while(1){
        Serial.println("{\"error\" : \"ERROR!\"}");
        sleep(1000);
    }
}

void init_hardwares()
{
    ledcSetup(LEDC_CHAN_PWM1, LEDC_FREQ, LEDC_RESO);
    ledcSetup(LEDC_CHAN_PWM2, LEDC_FREQ, LEDC_RESO);

    ledcAttachPin(IO_PWM1, LEDC_CHAN_PWM1);
    ledcAttachPin(IO_PWM2, LEDC_CHAN_PWM2);

    nexInit();
    // button_reset.attachPush(buttonResetCb);
    ledcWrite(LEDC_CHAN_PWM1, 210);
    ledcWrite(LEDC_CHAN_PWM2, 210);

    Wire.begin();
    Serial.begin(115200);
}

void init_check_sensors()
{
    if (!sht31.begin(I2C_ADDR_SHT21))
    {
        Serial.println("{\"error\" : \"SHT31 ERROR!\"}");
        fail();
    }

    if (sht31.isHeaterEnabled()) 
        Serial.println("{\"info\" : \"SHT31 Heater Enabled!\"}");
    else 
        Serial.println("{\"info\" : \"SHT31 Heater Disabled!\"}");

    if(!ads1.isConnected()){
        Serial.println("{\"error\" : \"ADS 1 ERROR!\"}");
        fail();
    }

    if(!ads2.isConnected()){
        Serial.println("{\"error\" : \"ADS 2 ERROR!\"}");
        fail();
    }

    ads1.setGain(ADS_GAIN);
    ads2.setGain(ADS_GAIN);

    // Setting param gas sensors

    float mq135_rL = 20;
    float ratio_mq135_air = 3.6;
    
    mq135.setRegressionMethod(1);
    mq135.setRL(mq135_rL);

    float mq136_rL = 20;
    float ratio_mq136_air = 3.6;

    mq136.setRegressionMethod(1);
    mq136.setRL(mq136_rL);

    float mq137_rL = 20;
    float ratio_mq137_air = 3.6;

    mq137.setRegressionMethod(1);
    mq137.setRL(mq137_rL);

    float mq138_rL = 20;
    float ratio_mq138_air = 9.8;

    mq138.setRegressionMethod(1);
    mq138.setRL(mq138_rL);

    float mq2_rL = 20;
    float ratio_mq2_air = 9.83;

    mq2.setRegressionMethod(1);
    mq2.setRL(mq2_rL);

    float mq3_rL = 20;
    float ratio_mq3_air = 60;

    mq3.setRegressionMethod(1);
    mq3.setRL(mq3_rL);

    float tgs2620_rL = 20;
    float ratio_tgs2620_air = 21; // ini jg gatau bnr ga

    tgs2620.setRL(tgs2620_rL);
    tgs2620.setRatioAir(ratio_tgs2620_air);

    float tgs822_rL = 20;
    float ratio_tgs822_air = 17; //sepertinya

    tgs822.setRL(tgs822_rL);
    tgs822.setRatioAir(ratio_tgs822_air);


    pref.begin(PREF_NAMESPACE_ENOSE_R0, false); // load enose r0 pref

    #if IS_CALIBRATING_GAS_SENSOR
    Serial.println("{\"info\" : \"Calibrating gas sensors...\"}");

    // MQ135 Calibration
    float calc_r0 = 0;
    
    for(int i = 1; i<=10; i ++)
    {
        adc_mq135 = ads1.readADC(ADS1_CHAN_MQ135);
        mq135.setADC(adc_mq135);
        calc_r0 += mq135.calibrate(ratio_mq135_air);
    }
    mq135.setR0(calc_r0/mq135_rL);
    pref.putFloat("mq135", calc_r0);

    // if(isinf(calcR0)) {Serial.println("Warning: Conection issue, R0 is infinite (Open circuit detected) please check your wiring and supply"); while(1);}
    // if(calcR0 == 0){Serial.println("Warning: Conection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply"); while(1);}

    // MQ136 Calibration
    calc_r0 = 0;
    
    for(int i = 1; i<=10; i ++)
    {
        adc_mq136 = ads2.readADC(ADS2_CHAN_MQ136);
        mq136.setADC(adc_mq136);
        calc_r0 += mq136.calibrate(ratio_mq136_air);
    }
    mq136.setR0(calc_r0/mq136_rL);
    pref.putFloat("mq136", calc_r0);

    // MQ137 Calibration
    calc_r0 = 0;
    
    for(int i = 1; i<=10; i ++)
    {
        adc_mq137 = ads1.readADC(ADS1_CHAN_MQ137);
        mq137.setADC(adc_mq137);
        calc_r0 += mq137.calibrate(ratio_mq137_air);
    }
    mq137.setR0(calc_r0/mq137_rL);
    pref.putFloat("mq137", calc_r0);

    // MQ138 Calibration
    calc_r0 = 0;
    
    for(int i = 1; i<=10; i ++)
    {
        adc_mq138 = ads1.readADC(ADS1_CHAN_MQ138);
        mq138.setADC(adc_mq138);
        calc_r0 += mq138.calibrate(ratio_mq138_air);
    }
    mq138.setR0(calc_r0/mq138_rL);
    pref.putFloat("mq138", calc_r0);

    // MQ2 Calibration
    calc_r0 = 0;
    
    for(int i = 1; i<=10; i ++)
    {
        adc_mq2 = ads2.readADC(ADS2_CHAN_MQ2);
        mq2.setADC(adc_mq2);
        calc_r0 += mq2.calibrate(ratio_mq2_air);
    }
    mq2.setR0(calc_r0/mq2_rL);
    pref.putFloat("mq2", calc_r0);

    // MQ3 Calibration
    calc_r0 = 0;
    
    for(int i = 1; i<=10; i ++)
    {
        adc_mq3 = ads1.readADC(ADS1_CHAN_MQ3);
        mq3.setADC(adc_mq3);
        calc_r0 += mq3.calibrate(ratio_mq3_air);
    }
    mq3.setR0(calc_r0/mq3_rL);
    pref.putFloat("mq3", calc_r0);

    // TGS2620 Calibration // entah caranya bener ga
    calc_r0 = 0;

    for(int i = 1; i<=10; i ++)
    {
        adc_tgs2620 = ads2.readADC(ADS2_CHAN_TGS2620);
        tgs2620.setADC(adc_tgs2620);
        calc_r0 += tgs2620.calibrate(ratio_tgs2620_air);
    }
    tgs2620.setR0(calc_r0/tgs2620_rL);
    pref.putFloat("tgs2620", calc_r0);

    // TGS822 Calibration 
    calc_r0 = 0;

    for(int i = 1; i<=10; i ++)
    {
        adc_tgs822 = ads2.readADC(ADS2_CHAN_TGS822);
        tgs822.setADC(adc_tgs822);
        calc_r0 += tgs822.calibrate(ratio_tgs822_air);
    }
    tgs822.setR0(calc_r0/tgs822_rL);
    pref.putFloat("tgs822", calc_r0);

    Serial.println("{\"info\" : \"calibration done\"}");

    #else // get config from pref
    float mq135_r0    = pref.getFloat("mq135");
    float mq136_r0    = pref.getFloat("mq136");
    float mq137_r0    = pref.getFloat("mq137");
    float mq138_r0    = pref.getFloat("mq138");
    float mq2_r0      = pref.getFloat("mq2");
    float mq3_r0      = pref.getFloat("mq3");
    float tgs2620_r0  = pref.getFloat("tgs2620");
    float tgs822_r0   = pref.getFloat("tgs822");

    mq135.setR0   (mq135_r0 / mq135_rL);
    mq136.setR0   (mq136_r0 / mq136_rL);
    mq137.setR0   (mq137_r0 / mq137_rL);
    mq138.setR0   (mq138_r0 / mq138_rL);
    mq2.setR0     (mq2_r0 / mq2_rL);
    mq3.setR0     (mq3_r0 / mq3_rL);
    tgs2620.setR0 (tgs2620_r0 / tgs2620_rL);
    tgs822.setR0  (tgs822_r0 / tgs822_rL);

    #endif

    pref.end();

}

void init_tasks()
{
    // xTaskCreatePinnedToCore(
    //     Core0Task,   
    //     "Core0Task", 
    //     10000,    
    //     NULL,     
    //     1,        
    //     &t0H,     
    //     0);


    sch.init();
    sch.addTask(task_ads);
    sch.addTask(task_nex);
    task_ads.enable();
    task_nex.enable();
}

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

    // calculate ppm
    // MQ135 ======================================================================================================================================
    /* Sauce : github MQUnifiedsensor
        Exponential regression:
        GAS      | a      | b
        CO       | 605.18 | -3.937  
        Alcohol  | 77.255 | -3.18 
        CO2      | 110.47 | -2.862
        Toluen   | 44.947 | -3.445
        NH4      | 102.2  | -2.473
        Aceton   | 34.668 | -3.369
    */

    mq135.setADC(adc_mq135);

    mq135.setA(605.18); mq135.setB(-3.937); // Configure the equation to calculate CO concentration value
    float mq135_co = mq135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

    mq135.setA(77.255); mq135.setB(-3.18); //Alcohol
    float mq135_alcohol = mq135.readSensor(); 

    mq135.setA(110.47); mq135.setB(-2.862); // CO2
    float mq135_co2 = mq135.readSensor(); 

    mq135.setA(44.947); mq135.setB(-3.445); // Toluen
    float mq135_toluen = mq135.readSensor(); 
    
    mq135.setA(102.2 ); mq135.setB(-2.473); // NH4
    float mq135_nh4 = mq135.readSensor(); 

    mq135.setA(34.668); mq135.setB(-3.369); // Aceton
    float mq135_aceton = mq135.readSensor(); 

    // MQ136 ======================================================================================================================================
      /*
        Exponential regression:
        GAS      | a      | b
        H2S      | 36.737 | -3.536
        NH4      | 98.551 | -2.475
        CO       | 503.34 | -3.774
    */
    mq136.setADC(adc_mq136);

    mq136.setA(503.34); mq136.setB(-3.774); //CO
    float mq136_co = mq136.readSensor(); 

    mq136.setA(98.551); mq136.setB(-2.475); //NH4
    float mq136_nh4 = mq136.readSensor(); 

    mq136.setA(36.737); mq136.setB(-3.536); //H2S
    float mq136_h2s = mq136.readSensor(); 

    // MQ137 ======================================================================================================================================
      /*
        Exponential regression:
        GAS      | a      | b
        NH3      | 36.737 | -3.536
        Ethanol  | 98.551 | -2.475
        CO       | 503.34 | -3.774
    */
    mq137.setADC(adc_mq137);

    mq137.setA(503.34); mq137.setB(-3.774);
    float mq137_co = mq137.readSensor(); 

    mq137.setA(98.551); mq137.setB(-2.475);
    float mq137_ethanol = mq137.readSensor(); 

    mq137.setA(36.737); mq137.setB(-3.536);
    float mq137_nh3 = mq137.readSensor(); 

    // MQ138 ======================================================================================================================================
      /*
        Exponential regression:
        Gas       | a      | b
        Benzene   | 987.99 | -2.162
        Hexane    | 574.25 | -2.222
        CO        | 36974  | -3.109
        Alcohol   | 3616.1 | -2.675
        Propane   | 658.71 | -2.168
    */
    mq138.setADC(adc_mq138);

    mq138.setA(987.99); mq138.setB(-2.162);
    float mq138_benzene = mq138.readSensor(); 
    
    mq138.setA(574.25); mq138.setB(-2.222);
    float mq138_hexane = mq138.readSensor();

    mq138.setA(36974); mq138.setB(-3.109);
    float mq138_co = mq138.readSensor();

    mq138.setA(3616.1); mq138.setB(-2.675);
    float mq138_alcohol = mq138.readSensor(); 

    mq138.setA(658.71); mq138.setB(-2.168);
    float mq138_propane = mq138.readSensor(); 

    // MQ2 ======================================================================================================================================
      /*
        Exponential regression:
        Gas    | a      | b
        H2     | 987.99 | -2.162
        LPG    | 574.25 | -2.222
        CO     | 36974  | -3.109
        Alcohol| 3616.1 | -2.675
        Propane| 658.71 | -2.168
    */
    mq2.setADC(adc_mq2);

    mq2.setA(987.99); mq2.setB(-2.162);
    float mq2_h2 = mq2.readSensor(); 
    
    mq2.setA(574.25); mq2.setB(-2.222);
    float mq2_lpg = mq2.readSensor();

    mq2.setA(36974); mq2.setB(-3.109);
    float mq2_co = mq2.readSensor();

    mq2.setA(3616.1); mq2.setB(-2.675);
    float mq2_alcohol = mq2.readSensor(); 

    mq2.setA(658.71); mq2.setB(-2.168);
    float mq2_propane = mq2.readSensor(); 

    // MQ3 ======================================================================================================================================
    /*
        Exponential regression:
        Gas    | a      | b
        LPG    | 44771  | -3.245
        CH4    | 65.8824| -1.157803532
        CO     | 521853 | -3.821
        Alcohol| 0.3934 | -1.504
        Benzene| 4.8387 | -2.68
        Hexane | 7585.3 | -2.849
    */
    mq3.setADC(adc_mq3);

    mq3.setA(44771); mq3.setB(-3.245);
    float mq3_lpg = mq3.readSensor(); 

    mq3.setA(65.8824); mq3.setB(-1.1578);
    double mq3_ch4 = mq3.readSensor(); 

    mq3.setA(521853); mq3.setB(-3.821);
    float mq3_co = mq3.readSensor(); 

    mq3.setA(0.3934); mq3.setB(-1.504);
    float mq3_alcohol = mq3.readSensor(); 

    mq3.setA(4.8387); mq3.setB(-2.68);
    float mq3_benzene = mq3.readSensor(); 

    mq3.setA(7585.3); mq3.setB(-2.849);
    float mq3_hexane = mq3.readSensor(); 

    // TGS822 ======================================================================================================================================
    /*
        Exponential regression:
        Gas         | a             | b
        Methane     | 801945        | -3.583947
        CO          | 2966.6364     | -2.417324
        Isobutane   | 1424.408      | -2.263743
        Hexane      | 343.0545      | -1.763533
        Benzene     | 353.6719      | -1.575331
        Ethanol     | 282.5765      | -1.599635
        Acetone     | 317.8024      | -1.270728

    */
    tgs822.setADC(adc_tgs822);

    double tgs822_methane    = tgs822.calculatePpm(adc_tgs822, 801945, -3.583947);
    float tgs822_co         = tgs822.calculatePpm(adc_tgs822, 2966.6364, -2.417324);
    float tgs822_isobutane  = tgs822.calculatePpm(adc_tgs822, 1424.408, -2.263743);
    float tgs822_hexane     = tgs822.calculatePpm(adc_tgs822, 343.0545, -1.763533);
    float tgs822_benzene    = tgs822.calculatePpm(adc_tgs822, 353.6719, -1.575331);
    float tgs822_ethanol    = tgs822.calculatePpm(adc_tgs822, 282.5765, -1.599635);
    float tgs822_acetone    = tgs822.calculatePpm(adc_tgs822, 317.8024, -1.270728);

    // Serial.println(tgs822_methane);

    // TGS2620 ======================================================================================================================================
    /*
        Exponential regression:
        Gas         | a             | b
        Methane     | 24599.121     | -2.1599
        CO          | 1301.825      | -1.7327
        Isobutane   | 495.948       | -1.5260
        H2          | 334.361       | -1.8144
        Ethanol     | 324.036       | -1.50063
        
    */

    tgs2620.setADC(adc_tgs2620);

    float tgs2620_methane     = tgs2620.calculatePpm(adc_tgs2620, 24599.121, -2.1599);
    float tgs2620_co          = tgs2620.calculatePpm(adc_tgs2620, 1301.825, -1.7327);
    float tgs2620_isobutane   = tgs2620.calculatePpm(adc_tgs2620, 495.948, -1.5260);
    float tgs2620_h2          = tgs2620.calculatePpm(adc_tgs2620, 334.361, -1.8144);
    float tgs2620_ethanol     = tgs2620.calculatePpm(adc_tgs2620, 324.036, -1.50063);

    // SHT31 sensor readings
    float t = sht31.readTemperature();
    float h = sht31.readHumidity();

    if (isnan(t)){t = -99;}
    if (isnan(h)){h = -99;}

    // send data to m6
    #if USE_PPM
    #define SENSOR_DATA_SIZE 10 + 6 + 3 + 3 + 5 + 5 + 6 + 7 + 5
    #else
    #define SENSOR_DATA_SIZE 10
    #endif

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

    #if USE_PPM
    sensor_json["mq135_co"]      = mq135_co;
    sensor_json["mq135_alcohol"] = mq135_alcohol;
    sensor_json["mq135_co2"]     = mq135_co2;
    sensor_json["mq135_toluen"]  = mq135_toluen;
    sensor_json["mq135_nh4"]     = mq135_nh4;
    sensor_json["mq135_aceton"]  = mq135_aceton;

    sensor_json["mq136_co"]  = mq136_co;
    sensor_json["mq136_nh4"] = mq136_nh4;
    sensor_json["mq136_h2s"] = mq136_h2s;

    sensor_json["mq137_co"]       = mq137_co;
    sensor_json["mq137_ethanol"]  = mq137_ethanol;
    sensor_json["mq137_nh3"]      = mq137_nh3;

    sensor_json["mq138_benzene"]  = mq138_benzene;
    sensor_json["mq138_hexane"]   = mq138_hexane;
    sensor_json["mq138_co"]       = mq138_co;
    sensor_json["mq138_alcohol"]  = mq138_alcohol;
    sensor_json["mq138_propane"]  = mq138_propane;

    sensor_json["mq2_h2"]       = mq2_h2;
    sensor_json["mq2_lpg"]      = mq2_lpg;
    sensor_json["mq2_co"]       = mq2_co;
    sensor_json["mq2_alcohol"]  = mq2_alcohol;
    sensor_json["mq2_propane"]  = mq2_propane;

    sensor_json["mq3_lpg"]      = mq3_lpg;
    sensor_json["mq3_ch4"]      = mq3_ch4;
    sensor_json["mq3_co"]       = mq3_co;
    sensor_json["mq3_alcohol"]  = mq3_alcohol;
    sensor_json["mq3_benzene"]  = mq3_benzene;
    sensor_json["mq3_hexane"]   = mq3_hexane;

    sensor_json["tgs822_methane"]    = tgs822_methane;
    sensor_json["tgs822_co"]         = tgs822_co;
    sensor_json["tgs822_isobutane"]  = tgs822_isobutane;
    sensor_json["tgs822_hexane"]     = tgs822_hexane;
    sensor_json["tgs822_benzene"]    = tgs822_benzene;
    sensor_json["tgs822_ethanol"]    = tgs822_ethanol;
    sensor_json["tgs822_acetone"]    = tgs822_acetone;

    sensor_json["tgs2620_methane"]   = tgs2620_methane;
    sensor_json["tgs2620_co"]        = tgs2620_co;
    sensor_json["tgs2620_isobutane"] = tgs2620_isobutane;
    sensor_json["tgs2620_h2"]        = tgs2620_h2;
    sensor_json["tgs2620_ethanol"]   = tgs2620_ethanol;
    #endif

    serializeJson(sensor_json, Serial);
    Serial.print("\n");

    #if USE_NEXTION_GRAPH
    //update graphs in nextion
    wave1.addValue(NEX_WAVE1_CHAN_MQ135,   ADC16TO8 * adc_mq135);
    wave1.addValue(NEX_WAVE1_CHAN_MQ136,   ADC16TO8 * adc_mq136);
    wave1.addValue(NEX_WAVE1_CHAN_MQ137,   ADC16TO8 * adc_mq137);
    wave1.addValue(NEX_WAVE1_CHAN_MQ138,   ADC16TO8 * adc_mq138);
    wave2.addValue(NEX_WAVE2_CHAN_MQ2,     ADC16TO8 * adc_mq2);
    wave2.addValue(NEX_WAVE2_CHAN_MQ3,     ADC16TO8 * adc_mq3);
    wave2.addValue(NEX_WAVE2_CHAN_TGS822,  ADC16TO8 * adc_tgs822);
    wave2.addValue(NEX_WAVE2_CHAN_TGS2620, ADC16TO8 * adc_tgs2620);
    #endif
}

void nexCallback()
{
    // nexLoop(nex_listen_list);
}

// void buttonResetCb(void *ptr)
// {
//   Serial.println("OFF1");
// }