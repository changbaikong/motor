#include "HardwareSerial.h"
#include <Arduino.h>
#include "Blue-smooth.h"

 void initBlue() {
    Serial2.begin(9600);    // 蓝牙串口
    Serial.print("正在启动蓝牙模式！");
    Serial.print("按下W前进,按下s后退,按下x停止.");
  }

  char Rx() {
    if (Serial2.available()) {
      char cmd = Serial2.read();
      switch (cmd) {
        case 'w': Serial.println("正转前进"); break;
        case 's': Serial.println("后退");     break;
        case 'x': Serial.println("停止");     break;
      }
      return cmd;
    }
    return -1;
  }
  void Tx(){
    if (Serial.available()) {
      Serial2.write(Serial.read());
    }
  }