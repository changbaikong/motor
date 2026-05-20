#include "motor.h"                         // 电机驱动库
#include <TimerOne.h>                       // Mega 2560 定时器中断库（库管理器搜 TimerOne 安装）

// ============================================================
// 编码器引脚定义
// Mega 2560 支持的外部中断引脚：2, 3, 18, 19, 20, 21
// 用 CHANGE 模式触发，每路编码器接 2 个中断引脚
// ============================================================
#define L_ENA 2                             // 左编码器 A 相 — 中断引脚
#define L_ENB 3                             // 左编码器 B 相 — 中断引脚
#define R_ENA 18                            // 右编码器 A 相 — 中断引脚
#define R_ENB 19                            // 右编码器 B 相 — 中断引脚

// ============================================================
// 编码器脉冲计数器（volatile：ISR 和主循环共享，禁止编译器优化）
// 1040 表示编码器每圈脉冲数 = 电机轴编码器线数 × 减速比
// ============================================================
volatile long L_EN_CNT = 0;                 // 左轮累计脉冲
volatile long R_EN_CNT = 0;                 // 右轮累计脉冲
long L_EN_LastCNT = 0;                      // 左轮上次脉冲（用于计算速度）
long R_EN_LastCNT = 0;                      // 右轮上次脉冲

// ============================================================
// 定时器 ISR 计算结果缓存（在 ISR 里算好，loop() 里打印）
// ============================================================
double LAngle = 0, RAngle = 0;              // 角度（度）
double LSpeed = 0, RSpeed = 0;              // 速度（RPM）
float Ldistance_m = 0, Rdistance_m = 0;     // 距离（米）
volatile bool dataReady = false;            // 新数据就绪标志

// ============================================================
// 电机对象
// 参数：使能引脚, IN1, IN2, 线序反转（true=已反转）
// ============================================================
motor motorB(9, 6, 7, false);               // B电机：PWM=9, IN1=6, IN2=7
motor motorC(10, 11, 12, false);            // C电机：PWM=10, IN1=11, IN2=12

// ============================================================
// 编码器 A 相中断：通过 A/B 相 XOR 判断旋转方向
// 原理：AB 相相差 90°，A 相跳变时 B 相电平不同 = 方向不同
// ============================================================
void L_ENA_IRQ() {
  if (digitalRead(L_ENA) ^ digitalRead(L_ENB)) {
    L_EN_CNT--;                             // 反转：脉冲减
  } else {
    L_EN_CNT++;                             // 正转：脉冲加
  }
}

void L_ENB_IRQ() {
  if (digitalRead(L_ENA) ^ digitalRead(L_ENB)) {
    L_EN_CNT++;
  } else {
    L_EN_CNT--;
  }
}

void R_ENA_IRQ() {
  if (digitalRead(R_ENA) ^ digitalRead(R_ENB)) {
    R_EN_CNT--;
  } else {
    R_EN_CNT++;
  }
}

void R_ENB_IRQ() {
  if (digitalRead(R_ENA) ^ digitalRead(R_ENB)) {
    R_EN_CNT++;
  } else {
    R_EN_CNT--;
  }
}

// ============================================================
// 定时器中断（每 50ms 触发一次）
// 读取编码器计数器，计算角度、速度、行驶距离
// ============================================================
void timerISR() {
  // 原子读取 4 字节 long（AVR 上非原子，关中断防止 GPIO ISR 插进来改一半）
  noInterrupts();
  long lCnt = L_EN_CNT;
  long rCnt = R_EN_CNT;
  interrupts();

  // 角度计算：脉冲数 / 每圈脉冲 × 360°
  LAngle = lCnt * 360.0 / 1040;
  RAngle = rCnt * 360.0 / 1040;

  // 速度计算：(本次脉冲 - 上次脉冲) / 每圈脉冲 / 时间间隔 × 转换系数
  // 结果单位：RPM（转/分钟）
  LSpeed = (lCnt - L_EN_LastCNT) * 1000.0 * 60 / 1040 / 50;
  RSpeed = (rCnt - R_EN_LastCNT) * 1000.0 * 60 / 1040 / 50;

  // 距离计算：圈数 × 每圈周长
  // 151mm = 轮子周长（按实际轮径调整此值）
  long Ldistance_mm = (lCnt / 1040) * 151;
  Ldistance_m = Ldistance_mm / 1000.0;      // 转换为米
  long Rdistance_mm = (rCnt / 1040) * 151;
  Rdistance_m = Rdistance_mm / 1000.0;

  L_EN_LastCNT = lCnt;                      // 保存当前脉冲供下次速度计算
  R_EN_LastCNT = rCnt;

  dataReady = true;                         // 通知 loop() 有新数据
}

// ============================================================
// 初始化
// ============================================================
void setup() {
  // 编码器引脚：输入模式
  pinMode(L_ENA, INPUT);
  pinMode(L_ENB, INPUT);
  pinMode(R_ENA, INPUT);
  pinMode(R_ENB, INPUT);

  // 编码器中断：AB 相的每个跳变都触发，实现 4 倍频
  attachInterrupt(digitalPinToInterrupt(L_ENA), L_ENA_IRQ, CHANGE);
  attachInterrupt(digitalPinToInterrupt(L_ENB), L_ENB_IRQ, CHANGE);
  attachInterrupt(digitalPinToInterrupt(R_ENA), R_ENA_IRQ, CHANGE);
  attachInterrupt(digitalPinToInterrupt(R_ENB), R_ENB_IRQ, CHANGE);

  // Timer1 定时器：50ms 周期（50000μs）
  Timer1.initialize(50000);
  Timer1.attachInterrupt(timerISR);

  // 串口通信
  Serial.begin(115200);

  // 电机初始化
  motorB.begin();
  motorC.begin();

  delay(1000);                              // 等待系统稳定
}

// ============================================================
// 主循环：检测 dataReady 标志，有新数据就打印
// ============================================================
void loop() {
  if (dataReady) {
    noInterrupts();
    dataReady = false;
    interrupts();

    Serial.print("L Angle = ");
    Serial.print(LAngle);
    Serial.print("  R Angle = ");
    Serial.print(RAngle);
    Serial.print("  L speed = ");
    Serial.print(LSpeed);
    Serial.print("  R speed = ");
    Serial.print(RSpeed);
    Serial.print("  L distance = ");
    Serial.print(Ldistance_m);
    Serial.print("  R distance = ");
    Serial.println(Rdistance_m);
  }
}
