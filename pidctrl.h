#ifndef pidctrl_H
#define pidctrl_H

#include <Arduino.h>

#define target_rpm 150

// ★ KFF 沿用标定值，PID 还原最初版本
//   KD=0.10 配合 ÷0.05 等于 2× 增益，注意高频抖动

#define KFFA 0.0
#define KP_A 0.12
#define KI_A 0.06
#define KD_A 0.06

#define KFFB 0.0
#define KP_B 0.12
#define KI_B 0.06
#define KD_B 0.06

#define KFFC 0.0
#define KP_C 0.12
#define KI_C 0.06
#define KD_C 0.06

#define KFFD 0.0
#define KP_D 0.12
#define KI_D 0.06
#define KD_D 0.06


extern float iTermA, iTermB, iTermC, iTermD;
extern float   pwmA,  pwmB,  pwmC,  pwmD;
extern float lastSpeedA, lastSpeedB, lastSpeedC, lastSpeedD;

// 执行一轮 PID 计算，更新 pwm
// 参数：四个电机的当前 RPM
void pidCompute(double speedA, double speedB, double speedC, double speedD,float yawOffset);

// 重置积分项（停止时调用）
void pidReset();

#endif
