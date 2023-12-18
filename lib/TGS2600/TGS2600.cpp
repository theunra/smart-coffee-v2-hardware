// #include "TGS2600.h"

// /*****************************  getHydrogenGasPercentage **********************************
// Input:   rs_ro_ratio - Rs divided by Ro
// Output:  ppm of the target gas
// Remarks: This function passes different curves to the getPercentage function which 
//          calculates the ppm (parts per million) of the target gas.
// ************************************************************************************/ 
// int TGS2600::getHydrogenGasPercentage(float ro)
// {
//   return getPercentage(ro,H2_terCurve);
// }

// int TGS2600::getButaneGasPercentage(float ro)
// {
//   return getPercentage(ro,C4H10_Curve);
// }

// int TGS2600::getEthanolGasPercentage(float ro)
// {
//   return getPercentage(ro,C2H5OH_secCurve);
// }

// float TGS2600::calibrateInCleanAir(int raw_adc) {
//   SensorBase::calibrateInCleanAir(raw_adc, 1, H2_terCurve);
// }

#include "TGS2600.h"

void TGS2600::calculateRatio()
{
  VRL_F2600 = (analogRead(A3))*(5.0/1023);
  Rs_F2600 = ((2.0/VRL_F2600)-1)*RL_F2600;
  ratio_F2600 = Rs_F2600/Ro_F2600;
}

void TGS2600::calculateppm_CO()
{
  calculateRatio();
  float ratio = getRatio();
  ppm_CO = CO_A*pow(ratio, CO_B);
}

void TGS2600::calculateppm_CH4()
{
  calculateRatio();
  float ratio = getRatio();
  ppm_CH4 = CH4_A*pow(ratio, CH4_B);
}