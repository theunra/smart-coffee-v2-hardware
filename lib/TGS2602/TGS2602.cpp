
#include "TGS2602.h"


void TGS2602::calculateRatio(float adc){
  // VRL_F2602 = analogRead(F2602)*(5.0/1023.0); 
  VRL_F2602 = adc * (6.144/65536.0); 
  Rs_F2602 = ((6.144/VRL_F2602)-1)*(RL_F2602); 
  ratio_F2602 = Rs_F2602/Ro_F2602;
}
void TGS2602::calculateppm_NH3(float adc){
  calculateRatio(adc);
  float ratio_F2602 = getRatio();
  ppm_NH3 = NH3_A * pow(ratio_F2602, NH3_B);
}
void TGS2602::calculateppm_H2S(float adc){
  calculateRatio(adc);
  float ratio_F2602 = getRatio();
  ppm_H2S = H2S_A * pow(ratio_F2602, H2S_B);  
}
void TGS2602::calculateppm_VOC(float adc){
  calculateRatio(adc);
  float ratio_F2602 = getRatio();
  ppm_VOC = VOC_A * pow(ratio_F2602, VOC_B);
}
// /*****************************  getChlorineGasPercentage **********************************
// Input:   rs_ro_ratio - Rs divided by Ro
// Output:  ppm of the target gas
// Remarks: This function passes different curves to the getPercentage function which 
//          calculates the ppm (parts per million) of the target gas.
// ************************************************************************************/ 
// int TGS2602::getSewerGasPercentage(float ro)
// {
//   return getPercentage(ro,H2S_Curve);
// }

// int TGS2602::getTolueneGasPercentage(float ro)
// {
//   return getPercentage(ro,C7H8_Curve);
// }

// int TGS2602::getEthanolGasPercentage(float ro)
// {
//   return getPercentage(ro,C2H5OH_quarCurve);
// }

// int TGS2602::getAmmoniaGasPercentage(float ro)
// {
//   return getPercentage(ro,NH3_Curve);
// }

// float TGS2602::calibrateInCleanAir(int raw_adc) {
//   SensorBase::calibrateInCleanAir(raw_adc, 1, C7H8_Curve);
// }
