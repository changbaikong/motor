#include "encoder.h"
#include "pidctrl.h"
#include "btcontrol.h"
#include "serialout.h"
#include "imu.h"
#include "oled.h"
#include "imupid.h"

const int16_t GYRO_Z_DEADZONE = 30;  // 与 imu.cpp 保持一致

void setup() {
  encoderInit();
  timerCalcInit();
  btcontrolInit();
  initOLED();
  initIMU();
  yawctrlInit();

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
      yawctrComputer(yaw);
      pidCompute(ASpeed, BSpeed, CSpeed, DSpeed, diff_pwm);

      if (pwmA < 15) pwmA = 15;  if (pwmA > 255) pwmA = 255;
      if (pwmB < 15) pwmB = 15;  if (pwmB > 255) pwmB = 255;
      if (pwmC < 15) pwmC = 15;  if (pwmC > 255) pwmC = 255;
      if (pwmD < 15) pwmD = 15;  if (pwmD > 255) pwmD = 255;

      doMotor();
      plotterPrint();
    }

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
