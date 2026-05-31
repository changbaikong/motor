#include <math.h>
#ifndef IMUPID_H
#define IMUPID_H

#include <Arduino.h>

#define yaw_target 0.0
#define yaw_min -1.0
#define yaw_max 2.0


#define kp_yaw 1
#define ki_yaw 0.10
#define kd_yaw 0

#define i_yaw_limit -30.0
#define i_yaw_max 30.0

#define diff_yaw_limit -50.0
#define diff_yaw_max 50.0

extern float diff_pwm;
extern float yawRef;
void yawresetRef();


void yawctrlInit();
void yawctrComputer(float yaw);

#endif