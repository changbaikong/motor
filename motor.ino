#include "encoder.h"
#include "pidctrl.h"
#include "btcontrol.h"
#include "serialout.h"
#include "imu.h"
#include "oled.h"
#include "imupid.h"

const int16_t GYRO_Z_DEADZONE = 30;  // 与 imu.cpp 保持一致

void setup() {
  Serial.begin(115200);
  encoderInit();
  timerCalcInit();
  btcontrolInit();
  // initOLED();
  // initIMU();
  // yawctrlInit();


  Serial.println("=== 闭环测试模式（PWM=?，PID） ===");
  Serial.println("蓝牙模式: 按下w=正转 / 按下s=反转 / 按下x=停止");
  delay(1000);
}

void loop() {
  btcontrolRead();
  
  static int lastDirCmd = -1;

  if(cmd == 'A'){
    target_rpm += 20;
    if (target_rpm > 200)target_rpm = 200; //限制加速;
      Serial.print(">>>加速！ 目标 RPM =");
      Serial.print(target_rpm);
      cmd  = lastDirCmd;
  }
  else if (cmd == 'B'){
    target_rpm -= 20;
    if (target_rpm < 20) target_rpm = 20;
      Serial.print("减速!目标RPM=");
      Serial.print(target_rpm);
      cmd = lastDirCmd;
  }
  else if (cmd == 'w' || cmd == 's' || cmd == 'a' || cmd == 'd'){
    lastDirCmd = cmd;
  }
  else if (cmd == 'x'){
    lastDirCmd = -1;
  }
  if (dataReady) {
    noInterrupts();
    dataReady = false;
    interrupts();

    if (cmd != 'x' && cmd != -1) {
      // === PID 闭环计算（运动时才执行） ===
      pidCompute(ASpeed, BSpeed, CSpeed, DSpeed, 0, Adistance_m, Bdistance_m, Cdistance_m, Ddistance_m);

      if (pwmA < 0) pwmA = 0;  if (pwmA > 255) pwmA = 255;
      if (pwmB < 0) pwmB = 0;  if (pwmB > 255) pwmB = 255;
      if (pwmC < 0) pwmC = 0;  if (pwmC > 255) pwmC = 255;
      if (pwmD < 0) pwmD = 0;  if (pwmD > 255) pwmD = 255;
    }

    // doMotor 一定要放在 if 外面！
    // 因为按 'x' 停止时也需要调用 doMotor() 来执行 motor.stop()
    doMotor();
    plotterPrint();

    debugPrint(cmd);
  }
    if (!dmpReady) return;

  // ---- 1. 读传感器 ----
  if (!readSensors()) return;

  // ---- 2. 更新姿态 ----
  updatePose();
  updateYaw();

  // ---- 3. 输出 ----
  // Serial.print(yaw);
  // Serial.print("\t");
  // Serial.print(pitch_cf);
  // Serial.print("\t");
  // Serial.println(roll_cf);

  OLED.clearDisplay();
  OLED.setTextSize(1);
  OLED.setCursor(0, 0);

  OLED.print("Pitch: "); OLED.println(pitch_cf);
  OLED.print("Roll:  "); OLED.println(roll_cf);
  OLED.print("Yaw:   "); OLED.println(yaw);

  // if (abs(gyro_z_raw) < GYRO_Z_DEADZONE) {
  //   OLED.fillCircle(122, 58, 3, SSD1306_WHITE);
  // }
  OLED.display();

}
