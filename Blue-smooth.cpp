#include "HardwareSerial.h"
#include <Arduino.h>
#include "Blue-smooth.h"

 void initBlue() {
    // Serial2.begin(9600);  // 蓝牙已注释，改用 USB 串口
    Serial.println("键盘控制模式：w前进 / s后退 / x停止");
  }

  char Rx() {
    if (Serial.available()) {
      char cmd = Serial.read();
      switch (cmd) {
        case 'w': Serial.println("正转前进"); break;
        case 's': Serial.println("后退");     break;
        case 'x': Serial.println("停止");     break;
        case 'a': Serial.println("左转");     break;
        case 'd': Serial.println("右转");     break;
        case 'A': Serial.println("加速");     break;
        case 'B': Serial.println("减速");     break;
        // 过滤掉换行符和回车符，避免它们冒充指令
        case '\n':
        case '\r':
          return -1;
        default:
          Serial.print("未知指令: ");
          Serial.println(cmd);
          return -1;
      }
      return cmd;
    }
    return -1;
  }
  void Tx(){
    // Serial2.write(Serial.read());  // 蓝牙已注释
  }