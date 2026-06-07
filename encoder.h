#ifndef encoder_H
#define encoder_H

#include <Arduino.h>

// ============================================================
// 编码器引脚定义（AB = 后，CD = 前）
// 全部统一用 PCINT0（PORTB），8 条线一个 ISR，行为完全一致
//   A: 10=PB4=A相, 11=PB5=B相
//   B: 12=PB6=A相, 13=PB7=B相
//   C: 52=PB1=A相, 53=PB0=B相
//   D: 50=PB3=A相, 51=PB2=B相
// ============================================================
#define A_ENA 10
#define A_ENB 11
#define B_ENA 12
#define B_ENB 13
#define C_ENA 52
#define C_ENB 53
#define D_ENA 50
#define D_ENB 51

// 编码器脉冲数（每圈）= 编码器线数 × 4倍频 × 减速比
// JGB37-520 霍尔传感器 11PPR × 4倍频 × 1:56减速比 = ?
#define ENC_PPR 1300

// 轮子周长（mm）
#define WHEEL_CIRC_MM 204

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
