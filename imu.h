#ifndef IMU_H
#define IMU_H

#include "Arduino.h"
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"

// ---- 姿态角 ----
extern float pitch_cf;
extern float roll_cf;
extern float yaw;

// ---- OLED 死区指示灯用 ----
extern int16_t gyro_z_raw;

// ---- 状态 ----
extern bool dmpReady;

// ---- 初始化 ----
void initIMU();

// ---- 读取传感器（成功返回 true）----
bool readSensors();

// ---- 姿态更新 ----
void updatePose();   // 互补滤波 → pitch_cf / roll_cf
void updateYaw();    // DMP + 死区 + 漂移补偿 → yaw

#endif
