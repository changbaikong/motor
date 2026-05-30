#include "pidctrl.h"

float iTermA = 0, iTermB = 0, iTermC = 0, iTermD = 0;
float lastSpeedA = 0, lastSpeedB = 0, lastSpeedC = 0, lastSpeedD = 0;


  float pwmA = target_rpm*KFF;
  float pwmB = target_rpm*KFF;
  float pwmC = target_rpm*KFF;
  float pwmD = target_rpm*KFF;
  float filtSpeedA = 0, filtSpeedB = 0, filtSpeedC = 0, filtSpeedD = 0;


void pidCompute(double speedA, double speedB, double speedC, double speedD) {

  // === 低通滤波（α=0.3） ===
  filtSpeedA = 0.7 * filtSpeedA + 0.3 * abs(speedA);
  filtSpeedB = 0.7 * filtSpeedB + 0.3 * abs(speedB);
  filtSpeedC = 0.7 * filtSpeedC + 0.3 * abs(speedC);
  filtSpeedD = 0.7 * filtSpeedD + 0.3 * abs(speedD);


  float errA = target_rpm - filtSpeedA;
  float errB = target_rpm - filtSpeedB;
  float errC = target_rpm - filtSpeedC;
  float errD = target_rpm - filtSpeedD;


  // === 积分项：条件积分（小误差积、大误差冻结） ===
  // 改动：大误差时冻结 I，而不是清零。避免极限环振荡。
  if (abs(errA) < 15) {
    iTermA += errA;
  }
  // else: iTermA 保持不变（冻结），不是清零

  if (abs(errB) < 15) {
    iTermB += errB;
  }

  if (abs(errC) < 15) {
    iTermC += errC;
  }

  if (abs(errD) < 15) {
    iTermD += errD;
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

  // === 微分项：KD 从 0.07 → 0.04 ===
  // Δt = 0.05s (50ms 定时器周期)
  float dTermA = (-KD_A * (filtSpeedA - lastSpeedA) / 0.05f);
  float dTermB = (-KD_B * (filtSpeedB - lastSpeedB) / 0.05f);
  float dTermC = (-KD_C * (filtSpeedC - lastSpeedC) / 0.05f);
  float dTermD = (-KD_D * (filtSpeedD - lastSpeedD) / 0.05f);

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

  if (newPwmA > 255) { newPwmA = 255; if (iTermA > 0) iTermA = 0; }
  if (newPwmA < 15)  { newPwmA = 15;  if (iTermA < 0) iTermA = 0; }
  if (newPwmB > 255) { newPwmB = 255; if (iTermB > 0) iTermB = 0; }
  if (newPwmB < 15)  { newPwmB = 15;  if (iTermB < 0) iTermB = 0; }
  if (newPwmC > 255) { newPwmC = 255; if (iTermC > 0) iTermC = 0; }
  if (newPwmC < 15)  { newPwmC = 15;  if (iTermC < 0) iTermC = 0; }
  if (newPwmD > 255) { newPwmD = 255; if (iTermD > 0) iTermD = 0; }
  if (newPwmD < 15)  { newPwmD = 15;  if (iTermD < 0) iTermD = 0; }

  pwmA = newPwmA;
  pwmB = newPwmB;
  pwmC = newPwmC;
  pwmD = newPwmD;

  lastSpeedA = filtSpeedA;
  lastSpeedB = filtSpeedB;
  lastSpeedC = filtSpeedC;
  lastSpeedD = filtSpeedD;
}

void pidReset() {
  iTermA = 0;
  iTermB = 0;
  iTermC = 0;
  iTermD = 0;
  lastSpeedA = 0; lastSpeedB = 0; lastSpeedC = 0; lastSpeedD = 0;
  // 同时重置滤波值和 PWM 为前馈初始值，确保下次启动从干净状态出发
  filtSpeedA = 0; filtSpeedB = 0; filtSpeedC = 0; filtSpeedD = 0;
  pwmA = target_rpm * KFF;
  pwmB = target_rpm * KFF;
  pwmC = target_rpm * KFF;
  pwmD = target_rpm * KFF;
}
