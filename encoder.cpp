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

static void C_ENA_IRQ() {
  if (digitalRead(C_ENA) ^ digitalRead(C_ENB)) {
    C_EN_CNT--;
  } else {
    C_EN_CNT++;
  }
}

static void C_ENB_IRQ() {
  if (digitalRead(C_ENA) ^ digitalRead(C_ENB)) {
    C_EN_CNT++;
  } else {
    C_EN_CNT--;
  }
}

static void D_ENA_IRQ() {
  if (digitalRead(D_ENA) ^ digitalRead(D_ENB)) {
    D_EN_CNT--;
  } else {
    D_EN_CNT++;
  }
}

static void D_ENB_IRQ() {
  if (digitalRead(D_ENA) ^ digitalRead(D_ENB)) {
    D_EN_CNT++;
  } else {
    D_EN_CNT--;
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

  attachInterrupt(digitalPinToInterrupt(A_ENA), A_ENA_IRQ, CHANGE);
  attachInterrupt(digitalPinToInterrupt(A_ENB), A_ENB_IRQ, CHANGE);
  attachInterrupt(digitalPinToInterrupt(B_ENA), B_ENA_IRQ, CHANGE);
  attachInterrupt(digitalPinToInterrupt(B_ENB), B_ENB_IRQ, CHANGE);
  attachInterrupt(digitalPinToInterrupt(C_ENA), C_ENA_IRQ, CHANGE);
  attachInterrupt(digitalPinToInterrupt(C_ENB), C_ENB_IRQ, CHANGE);
  attachInterrupt(digitalPinToInterrupt(D_ENA), D_ENA_IRQ, CHANGE);
  attachInterrupt(digitalPinToInterrupt(D_ENB), D_ENB_IRQ, CHANGE);
}

void timerCalcInit() {
  Timer1.initialize(TIMER_US);
  Timer1.attachInterrupt(timerISR);
}
