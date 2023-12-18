// /*
//   TGS2600 Air Quality Sensor for the Particle Photon

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

// class TGS2600: public SensorBase {
//   public:
//     TGS2600(int sampling_frequency, int sampling_interval_ms)
//     : SensorBase(sampling_frequency, sampling_interval_ms, 1)
//     {
//     }
//     int getHydrogenGasPercentage(float ro);
//     int getButaneGasPercentage(float ro);
//     int getEthanolGasPercentage(float ro); 
//     float calibrateInCleanAir(int raw_adc); 
//     bool isCalibrate = true;   
//   private:
//     float H2_terCurve[2]  =  {0.3417050674, -2.887154835}; //TGS2600
//     float C4H10_Curve[2]   =  {0.3555567714, -3.337882361}; //TGS2600
//     float C2H5OH_secCurve[2] =  {0.2995093465,-3.148170562};//TGS2600
//     TGS2600() {};
// };

#include <Arduino.h>

#define RL_F2600 7.5

class TGS2600
{
private:
  float VRL_F2600; 
  float Rs_F2600; 
  float Ro_F2600 = 11.26;
  float ratio_F2600;

  float CO_A = 1.144997421;
  float CO_B = -0.21687423;
  float CH4_A = 1.05777824;
  float CH4_B = -0.0795645; 

  float ppm_CO;
  float ppm_CH4;

public:
   void calculateRatio();
   void calculateppm_CO();
   void calculateppm_CH4();

   //getter
   float getRatio() { return ratio_F2600; }
   float getppm_CO() {  return ppm_CO;  }
   float getppm_CH4() {  return ppm_CH4; }
};

