#include "pidctrl.h"
#include "imupid.h"
#include "encoder.h"

float iTermA = 0, iTermB = 0, iTermC = 0, iTermD = 0;
float lastSpeedA = 0, lastSpeedB = 0, lastSpeedC = 0, lastSpeedD = 0;

float pwmA = 0;   // 后轮响应猛，起步低
float pwmB = 0;
float pwmC = 0;   // 前轮需要多推一点
float pwmD = 0;

int target_rpm = 100;


void pidCompute(double speedA, double speedB, double speedC, double speedD,float yawOffset,double Adistance_m,double Bdistance_m,double Cdistance_m,double Ddistance_m ) {

  // // === 里程计纠偏：暂不启用 ===
  // float Leftdistance_m = (Adistance_m+Cdistance_m)/2;
  // float Rightdistance_m = (Bdistance_m+Ddistance_m)/2;
  // float localerr = Leftdistance_m-Rightdistance_m;
  // float odomCorrection = localerr * 5.0;
  // if (odomCorrection > 15)  odomCorrection = 15;
  // if (odomCorrection < -15) odomCorrection = -15;

  float fusedOffset = yawOffset;  // + odomCorrection;  // 偏航环+里程计均暂不启用

  float errA = (target_rpm+fusedOffset) - abs(speedA);
  float errB = (target_rpm-fusedOffset) - abs(speedB);
  float errC = (target_rpm+fusedOffset) - abs(speedC);
  float errD = (target_rpm-fusedOffset) - abs(speedD);


  // === 积分项：条件积分（小误差积、大误差冻结） ===
  // 改动：大误差时冻结 I，而不是清零。避免极限环振荡。
  if (abs(errA) < 15) {
    iTermA += errA * 0.5;
  }
  // else: iTermA 保持不变（冻结），不是清零

  if (abs(errB) < 15) {
    iTermB += errB * 0.5;
  }

  if (abs(errC) < 15) {
    iTermC += errC * 0.5;
  }

  if (abs(errD) < 15) {
    iTermD += errD * 0.5;
  }

  // iTerm 限幅从 ±25 → ±60，让积分有更大的调节范围
  if (iTermA > 60)  iTermA = 60;
  if (iTermA < -60) iTermA = -60;
  if (iTermB > 60)  iTermB = 60;
  if (iTermB < -60) iTermB = -60;
  if (iTermC > 60)  iTermC = 60;
  if (iTermC < -60) iTermC = -60;
  if (iTermD > 60)  iTermD = 60;
  if (iTermD < -60) iTermD = -60;

  // === 微分项：对测量值微分（Derivative on Measurement），避免 setpoint 变化 kick ===
  // Δt = 0.05s (50ms 定时器周期)
  float dTermA = (-KD_A * (abs(speedA) - lastSpeedA) / 0.05f);
  float dTermB = (-KD_B * (abs(speedB) - lastSpeedB) / 0.05f);
  float dTermC = (-KD_C * (abs(speedC) - lastSpeedC) / 0.05f);
  float dTermD = (-KD_D * (abs(speedD) - lastSpeedD) / 0.05f);

  // D 项输出限幅，防止编码器噪声引起高频抖动
  if (dTermA > 10)  dTermA = 10;
  if (dTermA < -10) dTermA = -10;
  if (dTermB > 10)  dTermB = 10;
  if (dTermB < -10) dTermB = -10;
  if (dTermC > 10)  dTermC = 10;
  if (dTermC < -10) dTermC = -10;
  if (dTermD > 10)  dTermD = 10;
  if (dTermD < -10) dTermD = -10;

  // === PID 输出 ===
  float deltaA = (KP_A * errA + KI_A * iTermA + dTermA);
  float deltaB = (KP_B * errB + KI_B * iTermB + dTermB);
  float deltaC = (KP_C * errC + KI_C * iTermC + dTermC);
  float deltaD = (KP_D * errD + KI_D * iTermD + dTermD);

  // delta 限幅从 ±25 → ±40，允许更快的动态响应
  if (deltaA > 40)  deltaA = 40;
  if (deltaA < -40) deltaA = -40;
  if (deltaB > 40)  deltaB = 40;
  if (deltaB < -40) deltaB = -40;
  if (deltaC > 40)  deltaC = 40;
  if (deltaC < -40) deltaC = -40;
  if (deltaD > 40)  deltaD = 40;
  if (deltaD < -40) deltaD = -40;


  // === 抗积分饱和：PWM 撞上限时清零同方向积分 ===
  float newPwmA = pwmA + deltaA;
  float newPwmB = pwmB + deltaB;
  float newPwmC = pwmC + deltaC;
  float newPwmD = pwmD + deltaD;

  newPwmA = 0.2 * pwmA + 0.8 * newPwmA;  // 降低滤波强度（0.5→0.2），加快响应
  newPwmB = 0.2 * pwmB + 0.8 * newPwmB;
  newPwmC = 0.2 * pwmC + 0.8 * newPwmC;
  newPwmD = 0.2 * pwmD + 0.8 * newPwmD;

  if (newPwmA > 255) { newPwmA = 255; if (iTermA > 0) iTermA = 0; }
  if (newPwmA < 0)  { newPwmA = 0;  if (iTermA < 0) iTermA = 0; }
  if (newPwmB > 255) { newPwmB = 255; if (iTermB > 0) iTermB = 0; }
  if (newPwmB < 0)  { newPwmB = 0;  if (iTermB < 0) iTermB = 0; }
  if (newPwmC > 255) { newPwmC = 255; if (iTermC > 0) iTermC = 0; }
  if (newPwmC < 0)  { newPwmC = 0;  if (iTermC < 0) iTermC = 0; }
  if (newPwmD > 255) { newPwmD = 255; if (iTermD > 0) iTermD = 0; }
  if (newPwmD < 0)  { newPwmD = 0;  if (iTermD < 0) iTermD = 0; }


  pwmA = newPwmA;
  pwmB = newPwmB;
  pwmC = newPwmC;
  pwmD = newPwmD;

  lastSpeedA = abs(speedA);
  lastSpeedB = abs(speedB);
  lastSpeedC = abs(speedC);
  lastSpeedD = abs(speedD);
}

void pidReset() {
  iTermA = 0;
  iTermB = 0;
  iTermC = 0;
  iTermD = 0;
  lastSpeedA = 0; lastSpeedB = 0; lastSpeedC = 0; lastSpeedD = 0;
  pwmA = KFFA * target_rpm;   // 100 × 1.3 = 130，起步就在目标附近
  pwmB = KFFB * target_rpm;
  pwmC = KFFC * target_rpm;
  pwmD = KFFD * target_rpm;
}
