#include "Arduino.h"
#include "imupid.h"
#include "imu.h"

static float iTerm=0;
float diff_pwm=0;
static unsigned long lastime=0;
float yawRef=0;

void yawctrComputer(float yaw){

unsigned long now = micros();
float dt =(now-lastime) / 1000000.0;
if (dt>0.1) dt = 0.1;
lastime=now;

// if (yaw>=yaw_min && yaw<= yaw_max){
//   diff_pwm *= 0.8;
//   iTerm *= 0.5;    // 快速衰减，留给一点点补偿就够了
//   return;
// }

float yaw_err = yawRef-yaw;

float P = kp_yaw * yaw_err;

iTerm += ki_yaw * yaw_err * dt;
iTerm = constrain(iTerm, i_yaw_limit, i_yaw_max);

float raw = P + iTerm;
raw = constrain(raw, diff_yaw_limit, diff_yaw_max);
diff_pwm = 0.2*diff_pwm + 0.8 * raw;


}
void yawresetRef(){
  yawRef=yaw;       // 保存当前航向角作为目标
  iTerm=0;          // 只重置积分项
  lastime=micros(); // 重新记录时间
  diff_pwm=0;       // 重置差速输出
}

void yawctrlInit(){
iTerm=0;
lastime=0;
diff_pwm=0;
yawRef=yaw;         // 初始化时也以当前方向为目标，而不是 0
}
