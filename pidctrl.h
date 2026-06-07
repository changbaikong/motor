#ifndef pidctrl_H
#define pidctrl_H

#include <Arduino.h>


extern int target_rpm;
#define wheel_base_mm 175
#define odom_P 5.0     // 里程计纠偏 P 增益（温和用 5，激进用 10）
// ★ KFF 沿用标定值，PID 还原最初版本
//   KD=0.10 配合 ÷0.05 等于 2× 增益，注意高频抖动

// 后轮(A/B)
#define KFFA 1.1
#define KP_A 0.32
#define KI_A 0.08
#define KD_A 0.0

#define KFFB 1.1
#define KP_B 0.32
#define KI_B 0.08
#define KD_B 0.0

// 前轮(C/D)
#define KFFC 1.1
#define KP_C 0.32
#define KI_C 0.08
#define KD_C 0.0

#define KFFD 1.1
#define KP_D 0.32
#define KI_D 0.08
#define KD_D 0.0


extern float iTermA, iTermB, iTermC, iTermD;
extern float   pwmA,  pwmB,  pwmC,  pwmD;
extern float lastSpeedA, lastSpeedB, lastSpeedC, lastSpeedD;

// 执行一轮 PID 计算，更新 pwm
// 参数：四个电机的当前 RPM
void pidCompute(double speedA, double speedB, double speedC, double speedD,
                float yawOffset,
                double Adistance_m, double Bdistance_m,
                double Cdistance_m, double Ddistance_m);

// 重置积分项（停止时调用）
void pidReset();

#endif
