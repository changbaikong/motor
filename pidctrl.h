#ifndef pidctrl_H
#define pidctrl_H

#include <Arduino.h>

#define target_rpm 150

// KP: 降低减少起步超调，KD: 增强阻尼抑制振荡，KI: 减半避免慢摆
#define KP_A 0.15
#define KI_A 0.06
#define KD_A 0.10

#define KP_B 0.15
#define KI_B 0.06
#define KD_B 0.10

#define KP_C 0.18
#define KI_C 0.06
#define KD_C 0.10

#define KP_D 0.18
#define KI_D 0.06
#define KD_D 0.10

// KFF: 稳态 PWM = target × KFF ≈ 82.5
#define KFF 0.55
#define KFF 0.55

extern float iTermA, iTermB, iTermC, iTermD;
extern float   pwmA,  pwmB,  pwmC,  pwmD;
extern float lastSpeedA, lastSpeedB, lastSpeedC, lastSpeedD;

// 执行一轮 PID 计算，更新 pwm
// 参数：四个电机的当前 RPM
void pidCompute(double speedA, double speedB, double speedC, double speedD);

// 重置积分项（停止时调用）
void pidReset();

#endif
