#include "SensorBase.h"
#include <Arduino.h>

#define F2602 A2

class TGS2602 {
  private:
    float VRL_F2602;
    float Rs_F2602;
    float RL_F2602 = 0.45;
    float Ro_F2602 = 59;
    float ratio_F2602;
    float NH3_A = 0.92372844;
    float NH3_B = -0.291578925;
    float H2S_A = 0.38881036;
    float H2S_B = -0.35010059;
    float VOC_A = 0.3220722;
    float VOC_B = -0.6007520;

    float ppm_NH3;
    float ppm_H2S;
    float ppm_VOC;
  public:
    void calculateRatio(float);
    void calculateppm_NH3(float);
    void calculateppm_H2S(float);
    void calculateppm_VOC(float);

    float getRatio(){return ratio_F2602;}
    float getppm_NH3(){return ppm_NH3;}
    float getppm_H2S(){return ppm_H2S;}
    float getppm_VOC(){return ppm_VOC;}
};


// /*
//   MQ131 Ozone Air Quality Sensor for the Spark Core

//   connect the sensor as follows :

//   A H A   >>> 5V
//   B   >>> A0
//   H       >>> GND
//   B       >>> 10K ohm >>> GND
  
//   Refactored into C++ class: David Warden Thomson
//   Contribution: epierre
//   Based on David Gironi http://davidegironi.blogspot.fr/2014/01/cheap-co2-meter-using-mq135-sensor-with.html

//   License: Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0)
 
// */
// #include "SensorBase.h"

// class TGS2602: public SensorBase {
//   public:
//     TGS2602(int sampling_frequency, int sampling_interval_ms)
//     : SensorBase(sampling_frequency, sampling_interval_ms, 1)
//     {
//     }
//     int getSewerGasPercentage(float ro);
//     int getTolueneGasPercentage(float ro);
//     int getEthanolGasPercentage(float ro); 
//     int getAmmoniaGasPercentage(float ro);
//     float calibrateInCleanAir(int raw_adc);
//     bool isCalibrate = true;
//   private:
//     float H2S_Curve[2]    =  {0.05566582614,-2.954075758}; //TGS2602     (0.8,0.1) (0.4,1) (0.25,3)
//     float NH3_Curve[2]  =  {0.92372844,  -3.448654502  }; //TGS2602    (0.8,1) (0.5,10) (0.3,30) 
//     TGS2602() {};
// };
