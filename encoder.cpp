#include "encoder.h"
#include <TimerOne.h>

// ============================================================
// 编码器脉冲计数器
// ============================================================
volatile long A_EN_CNT = 0;
volatile long B_EN_CNT = 0;
volatile long C_EN_CNT = 0;
volatile long D_EN_CNT = 0;

// ============================================================
// 定时器计算结果
// ============================================================
static long A_EN_LastCNT = 0;
static long B_EN_LastCNT = 0;
static long C_EN_LastCNT = 0;
static long D_EN_LastCNT = 0;

double ASpeed = 0, BSpeed = 0, CSpeed = 0, DSpeed = 0;
float Adistance_m = 0, Bdistance_m = 0, Cdistance_m = 0, Ddistance_m = 0;
volatile bool dataReady = false;

// ============================================================
// 四个编码器统一用 PCINT0 ISR（PORTB）
// 一次读 PINB，统一处理 8 个引脚，行为完全一致
//   A: PB4=pin10=A相, PB5=pin11=B相
//   B: PB6=pin12=A相, PB7=pin13=B相
//   C: PB1=pin52=A相, PB0=pin53=B相
//   D: PB3=pin50=A相, PB2=pin51=B相
// ============================================================
ISR(PCINT0_vect) {
  static uint8_t prevB = PINB;
  uint8_t curB = PINB;
  uint8_t chg = prevB ^ curB;  // 哪些引脚变了
  prevB = curB;

  // --- 编码器 A（PB4=A相, PB5=B相） ---
  if (chg & (1 << PB4)) {  // A相变化
    if ((curB & (1 << PB4)) ^ (curB & (1 << PB5))) A_EN_CNT--;
    else A_EN_CNT++;
  }
  if (chg & (1 << PB5)) {  // B相变化
    if ((curB & (1 << PB4)) ^ (curB & (1 << PB5))) A_EN_CNT++;
    else A_EN_CNT--;
  }

  // --- 编码器 B（PB6=A相, PB7=B相） ---
  if (chg & (1 << PB6)) {  // A相变化
    if ((curB & (1 << PB6)) ^ (curB & (1 << PB7))) B_EN_CNT--;
    else B_EN_CNT++;
  }
  if (chg & (1 << PB7)) {  // B相变化
    if ((curB & (1 << PB6)) ^ (curB & (1 << PB7))) B_EN_CNT++;
    else B_EN_CNT--;
  }

  // --- 编码器 C（PB1=A相, PB0=B相） ---
  if (chg & (1 << PB1)) {  // A相变化
    if ((curB & (1 << PB1)) ^ (curB & (1 << PB0))) C_EN_CNT--;
    else C_EN_CNT++;
  }
  if (chg & (1 << PB0)) {  // B相变化
    if ((curB & (1 << PB1)) ^ (curB & (1 << PB0))) C_EN_CNT++;
    else C_EN_CNT--;
  }

  // --- 编码器 D（PB3=A相, PB2=B相） ---
  if (chg & (1 << PB3)) {  // A相变化
    if ((curB & (1 << PB3)) ^ (curB & (1 << PB2))) D_EN_CNT--;
    else D_EN_CNT++;
  }
  if (chg & (1 << PB2)) {  // B相变化
    if ((curB & (1 << PB3)) ^ (curB & (1 << PB2))) D_EN_CNT++;
    else D_EN_CNT--;
  }
}

// ============================================================
// 定时器 ISR（每 50ms）：计算 RPM 和距离
// ============================================================
static void timerISR() {
  noInterrupts();
  long aCnt = A_EN_CNT;
  long bCnt = B_EN_CNT;
  long cCnt = C_EN_CNT;
  long dCnt = D_EN_CNT;
  interrupts();

  // 统一 PPR=2464（JGB37-520 1:56，四个编码器行为一致）
  ASpeed = (aCnt - A_EN_LastCNT) * 1000.0 * 60 / ENC_PPR / (TIMER_US / 1000);
  BSpeed = (bCnt - B_EN_LastCNT) * 1000.0 * 60 / ENC_PPR / (TIMER_US / 1000);
  CSpeed = (cCnt - C_EN_LastCNT) * 1000.0 * 60 / ENC_PPR / (TIMER_US / 1000);
  DSpeed = (dCnt - D_EN_LastCNT) * 1000.0 * 60 / ENC_PPR / (TIMER_US / 1000);

  Adistance_m = (aCnt  * WHEEL_CIRC_MM) / (ENC_PPR *1000.0);
  Bdistance_m = (bCnt  * WHEEL_CIRC_MM) / (ENC_PPR *1000.0);
  Cdistance_m = (cCnt  * WHEEL_CIRC_MM) / (ENC_PPR *1000.0);
  Ddistance_m = (dCnt  * WHEEL_CIRC_MM) / (ENC_PPR *1000.0);

  A_EN_LastCNT = aCnt;
  B_EN_LastCNT = bCnt;
  C_EN_LastCNT = cCnt;
  D_EN_LastCNT = dCnt;

  dataReady = true;
}

// ============================================================
// 初始化函数
// ============================================================
void encoderInit() {
  pinMode(A_ENA, INPUT_PULLUP);
  pinMode(A_ENB, INPUT_PULLUP);
  pinMode(B_ENA, INPUT_PULLUP);
  pinMode(B_ENB, INPUT_PULLUP);
  pinMode(C_ENA, INPUT_PULLUP);  // pin52=SCK，必须上拉防止 SPI 干扰
  pinMode(C_ENB, INPUT_PULLUP);  // pin53=SS，必须上拉防止误入 SPI 从模式
  pinMode(D_ENA, INPUT_PULLUP);  // pin50=MISO，也加上拉更稳
  pinMode(D_ENB, INPUT_PULLUP);

  // 全部编码器统一用 PCINT0（PORTB），不再使用硬件外部中断
  //   A: PB4(pin10) + PB5(pin11)
  //   B: PB6(pin12) + PB7(pin13)
  //   C: PB1(pin52) + PB0(pin53)
  //   D: PB3(pin50) + PB2(pin51)
  PCICR  |= (1 << PCIE0);
  PCMSK0 |= (1 << PCINT7) | (1 << PCINT6)   // B
          | (1 << PCINT5) | (1 << PCINT4)   // A
          | (1 << PCINT3) | (1 << PCINT2)   // D
          | (1 << PCINT1) | (1 << PCINT0);  // C
}

void timerCalcInit() {
  Timer1.initialize(TIMER_US);
  Timer1.attachInterrupt(timerISR);
}
