#ifndef btcontrol_H
#define btcontrol_H

#include <Arduino.h>
#include "motor.h"

// 电机对象
extern motor motorA;
extern motor motorB;
extern motor motorC;
extern motor motorD;

// 当前蓝牙命令（'w'/'s'/'x' / -1）
extern int cmd;

// 初始化蓝牙 + 四个电机
void btcontrolInit();

// 读取蓝牙命令，返回新命令或 -1（无新命令）
int btcontrolRead();

// 根据 cmd 和 pwm 驱动电机
void doMotor();

#endif
