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
// 编码器 A/B 相中断：通过 XOR 判断旋转方向
// ============================================================
static void A_ENA_IRQ() {
  if (digitalRead(A_ENA) ^ digitalRead(A_ENB)) {
    A_EN_CNT--;
  } else {
    A_EN_CNT++;
  }
}

static void A_ENB_IRQ() {
  if (digitalRead(A_ENA) ^ digitalRead(A_ENB)) {
    A_EN_CNT++;
  } else {
    A_EN_CNT--;
  }
}

static void B_ENA_IRQ() {
  if (digitalRead(B_ENA) ^ digitalRead(B_ENB)) {
    B_EN_CNT--;
  } else {
    B_EN_CNT++;
  }
}

static void B_ENB_IRQ() {
  if (digitalRead(B_ENA) ^ digitalRead(B_ENB)) {
    B_EN_CNT++;
  } else {
    B_EN_CNT--;
  }
}

// ============================================================
// 编码器 C/D 共用 PCINT0 ISR（PORTB）
//   C: 52=PB1=C_ENA, 53=PB0=C_ENB
//   D: 50=PB3=D_ENA, 51=PB2=D_ENB
// ============================================================
ISR(PCINT0_vect) {
  static uint8_t prevB = PINB;
  uint8_t curB = PINB;
  uint8_t chg = prevB ^ curB;
  prevB = curB;

  // --- 编码器 D（PB3=pin50=A相, PB2=pin51=B相） ---
  if (chg & (1 << PB3)) {
    if ((curB & (1 << PB3)) ^ (curB & (1 << PB2))) D_EN_CNT--;
    else D_EN_CNT++;
  }
  if (chg & (1 << PB2)) {
    if ((curB & (1 << PB3)) ^ (curB & (1 << PB2))) D_EN_CNT++;
    else D_EN_CNT--;
  }

  // --- 编码器 C（PB1=pin52=C_ENA=A相, PB0=pin53=C_ENB=B相） ---
  if (chg & (1 << PB0)) {  // C_ENB (B相) 跳变
    if ((curB & (1 << PB0)) ^ (curB & (1 << PB1))) C_EN_CNT++;
    else C_EN_CNT--;
  }
  if (chg & (1 << PB1)) {  // C_ENA (A相) 跳变
    if ((curB & (1 << PB0)) ^ (curB & (1 << PB1))) C_EN_CNT--;
    else C_EN_CNT++;
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

  ASpeed = (aCnt - A_EN_LastCNT) * 1000.0 * 60 / ENC_PPR / (TIMER_US / 1000);
  BSpeed = (bCnt - B_EN_LastCNT) * 1000.0 * 60 / ENC_PPR / (TIMER_US / 1000);
  CSpeed = (cCnt - C_EN_LastCNT) * 1000.0 * 60 / ENC_PPR / (TIMER_US / 1000);
  DSpeed = (dCnt - D_EN_LastCNT) * 1000.0 * 60 / ENC_PPR / (TIMER_US / 1000);

  Adistance_m = ((aCnt / ENC_PPR) * WHEEL_CIRC_MM) / 1000.0;
  Bdistance_m = ((bCnt / ENC_PPR) * WHEEL_CIRC_MM) / 1000.0;
  Cdistance_m = ((cCnt / ENC_PPR) * WHEEL_CIRC_MM) / 1000.0;
  Ddistance_m = ((dCnt / ENC_PPR) * WHEEL_CIRC_MM) / 1000.0;

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
  pinMode(A_ENA, INPUT);
  pinMode(A_ENB, INPUT);
  pinMode(B_ENA, INPUT);
  pinMode(B_ENB, INPUT);
  pinMode(C_ENA, INPUT);
  pinMode(C_ENB, INPUT);
  pinMode(D_ENA, INPUT);
  pinMode(D_ENB, INPUT);

  // A/B: 硬件外部中断（INT4/5, INT2/3）
  attachInterrupt(digitalPinToInterrupt(A_ENA), A_ENA_IRQ, CHANGE);
  attachInterrupt(digitalPinToInterrupt(A_ENB), A_ENB_IRQ, CHANGE);
  attachInterrupt(digitalPinToInterrupt(B_ENA), B_ENA_IRQ, CHANGE);
  attachInterrupt(digitalPinToInterrupt(B_ENB), B_ENB_IRQ, CHANGE);

  // C/D: 共用 PCINT0（PORTB）
  //   C: PB1(pin52) + PB0(pin53), D: PB3(pin50) + PB2(pin51)
  PCICR  |= (1 << PCIE0);
  PCMSK0 |= (1 << PCINT3) | (1 << PCINT2)   // D
          | (1 << PCINT1) | (1 << PCINT0);  // C
}

void timerCalcInit() {
  Timer1.initialize(TIMER_US);
  Timer1.attachInterrupt(timerISR);
}
