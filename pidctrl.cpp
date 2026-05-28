#include "pidctrl.h"

float iTermA = 0, iTermB = 0, iTermC = 0, iTermD = 0;
float   pwmA  = 0, pwmB  = 0, pwmC  = 0, pwmD  = 0;

void pidCompute(double speedA, double speedB, double speedC, double speedD) {
  float errA = target_rpm - abs(speedA);
  float errB = target_rpm - abs(speedB);
  float errC = target_rpm - abs(speedC);
  float errD = target_rpm - abs(speedD);


  
  if (abs(errA) < 15) iTermA += errA;   // error 在 ±15 以内才积分
  else               iTermA = 0;         // 大 error 时清零，纯 P 加速
  
  if (abs(errB) < 15) iTermB += errB;   // error 在 ±15 以内才积分
  else               iTermB = 0;         // 大 error 时清零，纯 P 加速
  
  if (abs(errC) < 15) iTermC += errC;   // error 在 ±15 以内才积分
  else               iTermC = 0;         // 大 error 时清零，纯 P 加速
  
  if (abs(errD) < 15) iTermD += errD;   // error 在 ±15 以内才积分
  else               iTermD = 0;         // 大 error 时清零，纯 P 加速

  if (iTermA > 50)  iTermA = 50;
  if (iTermA < -50) iTermA = -50;
  if (iTermB > 50)  iTermB = 50;
  if (iTermB < -50) iTermB = -50;
  if (iTermC > 50)  iTermC = 50;
  if (iTermC < -50) iTermC = -50;
  if (iTermD > 50)  iTermD = 50;
  if (iTermD < -50) iTermD = -50;


  float deltaA = (KP_A * errA + KI_A * iTermA);
  float deltaB = (KP_B * errB + KI_B * iTermB);
  float deltaC = (KP_C * errC + KI_C * iTermC);
  float deltaD = (KP_D * errD + KI_D * iTermD);

  if (deltaA > 50)  deltaA = 50;
  if (deltaA < -50) deltaA = -50;
  if (deltaB > 50)  deltaB = 50;
  if (deltaB < -50) deltaB = -50;
  if (deltaC > 50)  deltaC = 50;
  if (deltaC < -50) deltaC = -50;
  if (deltaD > 50)  deltaD = 50;
  if (deltaD < -50) deltaD = -50;


  pwmA += deltaA;
  pwmB += deltaB;
  pwmC += deltaC;
  pwmD += deltaD;

  if (pwmA > 255) pwmA = 255;
  if (pwmA < 15)  pwmA = 15;
  if (pwmB > 255) pwmB = 255;
  if (pwmB < 15)  pwmB = 15;
  if (pwmC > 255) pwmC = 255;
  if (pwmC < 15)  pwmC = 15;
  if (pwmD > 255) pwmD = 255;
  if (pwmD < 15)  pwmD = 15;
}

void pidReset() {
  iTermA = 0;
  iTermB = 0;
  iTermC = 0;
  iTermD = 0;
}
