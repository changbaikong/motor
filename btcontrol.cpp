#include "btcontrol.h"
#include "pidctrl.h"
#include "Blue-smooth.h"
#include "imupid.h"
#include "imu.h"

motor motorA(13, 26, 28, false);
motor motorB(10, 24, 22, false);
motor motorC(9, 30, 32, false);
motor motorD(8, 36, 34, false);

int cmd = -1;

void btcontrolInit() {
  initBlue();
  motorA.begin();
  motorB.begin();
  motorC.begin();
  motorD.begin();
}

int btcontrolRead() {
  int newCmd = Rx();
  if (newCmd != -1) cmd = newCmd;
  return cmd;
}

void doMotor() {
  static int lastCmd = -1;

  // 指令切换瞬间重置，不影响后续帧
  switch (cmd) {
    case 'x':
      if (lastCmd != 'x') { pidReset();yawresetRef(); }   // ← 只在按下瞬间执行一次
      motorA.stop();
      motorB.stop();
      motorC.stop();
      motorD.stop();
      break;
    case 'w':
      if (lastCmd != 'w') { yawresetRef(); }   // ← 只在按下瞬间执行一次
      motorA.forward(pwmA);
      motorB.forward(pwmB);
      motorC.forward(pwmC);
      motorD.forward(pwmD);
      break;
    case 's':
      if (lastCmd != 's') { yawresetRef(); }   // ← 只在按下瞬间执行一次
      motorA.backward(pwmA);
      motorB.backward(pwmB);
      motorC.backward(pwmC);
      motorD.backward(pwmD);
      break;
  }
  lastCmd = cmd;
}
