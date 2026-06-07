#include <math.h>
#ifndef IMUPID_H
#define IMUPID_H

#include <Arduino.h>

#define kp_yaw 3.1
#define ki_yaw 0.5
#define kd_yaw 0.5

#define i_yaw_limit -60.0
#define i_yaw_max 60.0

#define diff_yaw_limit -80.0
#define diff_yaw_max 80.0

extern float diff_pwm;
extern float yawRef;
void yawresetRef();


void yawctrlInit();
void yawctrComputer(float yaw);

#endif