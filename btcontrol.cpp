#include "btcontrol.h"
#include "pidctrl.h"
#include "Blue-smooth.h"

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
  switch (cmd) {
    case 'x':
      motorA.stop();
      motorB.stop();
      motorC.stop();
      motorD.stop();
      pidReset();
      break;
    case 'w':
      motorA.forward(pwmA);
      motorB.forward(pwmB);
      motorC.forward(pwmC);
      motorD.forward(pwmD);
      break;
    case 's':
      motorA.backward(pwmA);
      motorB.backward(pwmB);
      motorC.backward(pwmC);
      motorD.backward(pwmD);
      break;
  }
}
