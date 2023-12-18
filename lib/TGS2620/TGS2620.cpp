#include "TGS2620.h"

float TGS2620::calibrate(float ratio_clean_air){
  float RS_air; //Define variable for sensor resistance
  float R0; //Define variable for R0
  RS_air = ((_VOLT_RESOLUTION * RL) / VRL )-RL; //Calculate RS in fresh air
  if(RS_air < 0)  RS_air = 0; //No negative values accepted.
  R0 = RS_air / ratio_clean_air; //Calculate R0 
  if(R0 < 0)  R0 = 0; //No negative values accepted.
  return R0;
}
void TGS2620::setADC(int value)
{
  this-> VRL = (value) * _VOLT_RESOLUTION / ((pow(2, _ADC_Bit_Resolution)) - 1); 
  this-> _adc =  value;
}
void TGS2620::calculateRatio(float adc){
  VRL = adc * (_VOLT_RESOLUTION/_ADC_MAX_VALUE); 
  Rs = ((_VOLT_RESOLUTION/VRL) - ratio_in_air)*(RL); 
  ratio = Rs/Ro;
}

float TGS2620::calculatePpm(int adc, float a, float b){
  calculateRatio(adc); // sepertinya tdk perlu,
  float ratio = getRatio();
  float ppm = a * pow(ratio, b);

  if(ppm < 0) ppm = 0;

  return ppm;
}