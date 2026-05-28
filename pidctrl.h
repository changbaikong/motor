#ifndef pidctrl_H
#define pidctrl_H

#include <Arduino.h>

#define target_rpm 150

#define KP_A 0.4
#define KI_A 0.02

#define KP_B 0.3
#define KI_B 0.02

#define KP_C 0.4
#define KI_C 0.01

#define KP_D 0.4
#define KI_D 0.01

extern float iTermA, iTermB, iTermC, iTermD;
extern float   pwmA,  pwmB,  pwmC,  pwmD;

// 执行一轮 PID 计算，更新 pwm
// 参数：四个电机的当前 RPM
void pidCompute(double speedA, double speedB, double speedC, double speedD);

// 重置积分项（停止时调用）
void pidReset();

#endif
