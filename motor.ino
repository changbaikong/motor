#include "encoder.h"
#include "pidctrl.h"
#include "btcontrol.h"
#include "serialout.h"

void setup() {
  encoderInit();
  timerCalcInit();
  btcontrolInit();

  Serial.begin(115200);
  Serial.println("=== 电机测试就绪（AB后 / CD前） ===");
  Serial.println("蓝牙模式: 按下w=正转 / 按下s=反转 / 按下x=停止");
  delay(1000);
}

void loop() {
  btcontrolRead();
  doMotor();

  if (dataReady) {
    noInterrupts();
    dataReady = false;
    interrupts();

    if (cmd != 'x' && cmd != -1) {
      pidCompute(ASpeed, BSpeed, CSpeed, DSpeed);
      doMotor();
      plotterPrint();
    }

    debugPrint(cmd);
  }
}
