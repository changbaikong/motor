#include "motor.h"                         // 电机驱动库
#include <TimerOne.h>                       // Mega 2560 定时器中断库（库管理器搜 TimerOne 安装）

// ============================================================
// 编码器引脚定义
// Mega 2560 支持的外部中断引脚：2, 3, 18, 19, 20, 21
// ============================================================
#define L_ENA 2                             // 左编码器 A 相 — 中断引脚
#define L_ENB 3                             // 左编码器 B 相 — 中断引脚
#define R_ENA 19                            // 右编码器 A 相 — 中断引脚
#define R_ENB 18                            // 右编码器 B 相 — 中断引脚

// ============================================================
// 按钮引脚
// 接法：按钮一脚接引脚，另一脚接 GND（用 INPUT_PULLUP，按下=LOW）
// ============================================================
#define BTN_PIN 6

// ============================================================
// 编码器脉冲计数器（volatile：ISR 和主循环共享，禁止编译器优化）
// 1040 表示编码器每圈脉冲数 = 电机轴编码器线数 × 减速比
// ============================================================
volatile long L_EN_CNT = 0;                 // 左轮累计脉冲
volatile long R_EN_CNT = 0;                 // 右轮累计脉冲
long L_EN_LastCNT = 0;                      // 左轮上次脉冲（用于计算速度）
long R_EN_LastCNT = 0;                      // 右轮上次脉冲

// ============================================================
// 定时器 ISR 计算结果缓存
// ============================================================
double LAngle = 0, RAngle = 0;              // 角度（度）
double LSpeed = 0, RSpeed = 0;              // 速度（RPM）
float Ldistance_m = 0, Rdistance_m = 0;     // 距离（米）
volatile bool dataReady = false;            // 新数据就绪标志

// ============================================================
// 按钮去抖 & 状态机
// btnMode: 0=停止  1=正转  2=反转  （按一下换一个）
// ============================================================
int btnMode = 0;                            // 当前模式：0停 1正 2反
int btnStable = HIGH;                       // 确认后的状态（稳定值）
int btnLastReading = HIGH;                  // 上一次读到的原始值（用于检测跳变）
unsigned long btnLastTime = 0;              // 最后一次跳变的时间（去抖用）
int printTick = 0;                          // 打印计数器，每10次(500ms)才打印一次编码器

// ============================================================
// 电机对象
// 参数：使能引脚, IN1, IN2, 线序反转（true=已反转）
// ============================================================
motor motorA(13, 26, 28, false);            // A电机：PWM=13, IN1=26, IN2=28
motor motorB(10, 24, 22, false);            // B电机：PWM=10, IN1=24, IN2=22

#define target_rpm 150
#define KP 0.5
int pwmA = 0;
int pwmB = 0;



// ============================================================
// 编码器 A 相中断：通过 A/B 相 XOR 判断旋转方向
// 原理：AB 相相差 90°，A 相跳变时 B 相电平不同 = 方向不同
// ============================================================
void L_ENA_IRQ() {
  if (digitalRead(L_ENA) ^ digitalRead(L_ENB)) {
    L_EN_CNT--;
  } else {
    L_EN_CNT++;
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
// ============================================================
void timerISR() {
  noInterrupts();
  long lCnt = L_EN_CNT;
  long rCnt = R_EN_CNT;
  interrupts();

  LAngle = lCnt * 360.0 / 1040;
  RAngle = rCnt * 360.0 / 1040;

  LSpeed = (lCnt - L_EN_LastCNT) * 1000.0 * 60 / 1040 / 50;
  RSpeed = (rCnt - R_EN_LastCNT) * 1000.0 * 60 / 1040 / 50;

  long Ldistance_mm = (lCnt / 1040) * 151;
  Ldistance_m = Ldistance_mm / 1000.0;
  long Rdistance_mm = (rCnt / 1040) * 151;
  Rdistance_m = Rdistance_mm / 1000.0;

  L_EN_LastCNT = lCnt;
  R_EN_LastCNT = rCnt;

  dataReady = true;
}

// ============================================================
// 根据 btnMode 控制电机
// ============================================================
void doMotor() {
  switch (btnMode) {
    case 0:  // 停止
      motorA.stop();
      motorB.stop();
      break;
    case 1:  // 正转
      motorA.forward(pwmA);
      motorB.forward(pwmB);
      break;
    case 2:  // 反转
      motorA.backward(pwmA);
      motorB.backward(pwmB);
      break;
  }
}

// ============================================================
// 按钮检测（带去抖，检测"按下"那一瞬间）
// 原理：
//   btnLastReading — 每次 loop 都更新，用来发现电平跳变
//   btnStable — 只有稳定超过 50ms 后才更新，用来判断"真按下"
//   两个变量分开，就不会出现"证据被覆盖"的 bug
// ============================================================
void checkButton() {
  int reading = digitalRead(BTN_PIN);

  // 电平跳变了 → 重置计时器（可能是抖动，也可能是真按）
  if (reading != btnLastReading) {
    btnLastTime = millis();
  }

  // 稳定超过 50ms → reading 是可信的
  if (millis() - btnLastTime > 50) {
    // 可信值和之前不一样 → 真的变了
    if (reading != btnStable) {
      btnStable = reading;                  // 更新可信值

      // 按下（INPUT_PULLUP 模式，LOW = 按下）
      if (reading == LOW) {
        btnMode++;
        if (btnMode > 2) btnMode = 0;
        doMotor();

        Serial.println("");
        Serial.println("╔══════════════════════════╗");
        Serial.print("║  按钮按下! 模式 → ");
        if (btnMode == 0) Serial.println("停止  ║");
        else if (btnMode == 1) Serial.println("正转  ║");
        else Serial.println("反转  ║");
        Serial.println("╚══════════════════════════╝");
      }
    }
  }

  btnLastReading = reading;                 // 每次都要记，用于下轮比较
}

// ============================================================
// 初始化
// ============================================================
void setup() {
  pinMode(BTN_PIN, INPUT_PULLUP);           // 按钮：内部上拉，按下=LOW

  pinMode(L_ENA, INPUT);
  pinMode(L_ENB, INPUT);
  pinMode(R_ENA, INPUT);
  pinMode(R_ENB, INPUT);

  attachInterrupt(digitalPinToInterrupt(L_ENA), L_ENA_IRQ, CHANGE);
  attachInterrupt(digitalPinToInterrupt(L_ENB), L_ENB_IRQ, CHANGE);
  attachInterrupt(digitalPinToInterrupt(R_ENA), R_ENA_IRQ, CHANGE);
  attachInterrupt(digitalPinToInterrupt(R_ENB), R_ENB_IRQ, CHANGE);

  Timer1.initialize(50000);
  Timer1.attachInterrupt(timerISR);

  Serial.begin(115200);
  motorA.begin();
  motorB.begin();

  Serial.println("=== 电机测试就绪 ===");
  Serial.println("按钮模式: 按1下=正转 / 按2下=反转 / 按3下=停止");
  delay(1000);
}

// ============================================================
// 主循环
// ============================================================
void loop() {
  checkButton();                            // 每次循环都检查按钮进doMotor

  if (dataReady) {
    noInterrupts();
    dataReady = false;
    interrupts();

if(btnMode!=0){
    float errA = target_rpm - abs(LSpeed);
    float errB = target_rpm - abs(RSpeed);
    int deltaA = (int)(KP * errA);
    int deltaB = (int)(KP * errB);
    if(deltaA>50) deltaA=50;
    if(deltaA<-50) deltaA=-50;    // 加上负向限制
    if(deltaB>50) deltaB=50;
    if(deltaB<-50) deltaB=-50;    // 加上负向限制
    pwmA = pwmA + deltaA;
    pwmB = pwmB + deltaB;         // 修正：pwmB + deltaB
    if(pwmA > 255) pwmA = 255;
    if(pwmA < 15) pwmA = 15;
    if(pwmB > 255) pwmB = 255;
    if(pwmB < 15) pwmB = 15;
    doMotor();

    // 串口绘图器：三个通道（目标、左、右）
    Serial.print(target_rpm);
    Serial.print(",");
    Serial.print(LSpeed);
    Serial.print(",");
    Serial.println(RSpeed);
    }

    // 每 10 次（500ms）才打印一次编码器，避免刷屏
    printTick++;
    if (printTick >= 10) {
      printTick = 0;

      // 当前模式标签
      const char* modeStr = (btnMode == 0) ? "[停]" :
                            (btnMode == 1) ? "[正]" : "[反]";

      Serial.print(modeStr);
      Serial.print(" L RPM=");
      Serial.print(LSpeed);
      Serial.print("  R RPM=");
      Serial.print(RSpeed);
      Serial.print("  L dist=");
      Serial.print(Ldistance_m);
      Serial.print("  R dist=");
      Serial.println(Rdistance_m);
    }
  }
}
