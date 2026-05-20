#include "motor.h"                         // 包含电机驱动库的头文件

motor motorB(9, 6, 7);                     // B电机：PWMB=9, Bin1=6, Bin2=7
motor motorC(10, 11, 12);                  // C电机：PWMC=10, Cin1=11, Cin2=12
int anniu = 4;                             // 按钮连接的引脚号

void setup(){
  pinMode(anniu, INPUT_PULLUP);            // 设置按钮引脚为输入模式，启用内部上拉电阻
  Serial.begin(9600);                      // 初始化串口通信，波特率9600
  motorB.begin();                          // 初始化B电机
  motorC.begin();                          // 初始化C电机
  delay(1000);                             // 等待1秒让系统稳定
}

void loop(){
  

    }
  }
  lastButtonState = reading;
}
