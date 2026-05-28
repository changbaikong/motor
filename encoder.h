#ifndef encoder_H
#define encoder_H

#include <Arduino.h>

// ============================================================
// 编码器引脚定义（AB = 后，CD = 前）
// Mega 2560 支持的外部中断引脚：2, 3, 18, 19, 20, 21
// ============================================================
#define A_ENA 2
#define A_ENB 3
#define B_ENA 19
#define B_ENB 18
#define C_ENA 4
#define C_ENB 5
#define D_ENA 6
#define D_ENB 7

// 编码器脉冲数（每圈）= 编码器线数 × 减速比
#define ENC_PPR 1040

// 轮子周长（mm）
#define WHEEL_CIRC_MM 151

// 定时器间隔（us），50ms = 50000us
#define TIMER_US 50000

// ============================================================
// 编码器脉冲计数器（ISR 写入，主循环只读）
// ============================================================
extern volatile long A_EN_CNT;
extern volatile long B_EN_CNT;
extern volatile long C_EN_CNT;
extern volatile long D_EN_CNT;

// ============================================================
// 定时器计算结果（timerISR 写入，主循环只读）
// ============================================================
extern double ASpeed, BSpeed, CSpeed, DSpeed;     // RPM
extern float Adistance_m, Bdistance_m, Cdistance_m, Ddistance_m; // 米
extern volatile bool dataReady;

// ============================================================
// 初始化函数
// ============================================================
void encoderInit();   // pinMode + attachInterrupt
void timerCalcInit(); // Timer1 初始化 + 挂载 timerISR

#endif
