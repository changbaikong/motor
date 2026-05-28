#ifndef serialout_H
#define serialout_H

#include <Arduino.h>

// 串口绘图器输出（4 条速度曲线）
void plotterPrint();

// 调试打印（每 500ms 一次，输出 RPM + 距离）
void debugPrint(int cmd);

#endif
