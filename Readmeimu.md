

简写	结构体字段	物理量	单位	数据类型

a	acceleration	线性加速度	m/s² (米每二次方秒)	sensors\_vec\_t

w	gyro	角速度	rad/s (弧度每秒)	 sensors\_vec\_t

t	temperature	设备温度	°C (摄氏度) 	float





void initIMU(); 初始化IMU函数

void updateIMU();  更新IMU数据

void getEvent(); 获取AWT状态

float getRoll(); 获取roll值

float getPitch(); 获取pitch值

float getRollDisplay(); 展示给人看的ROLL函数

float getPitchDisplay(); 展示给人看的Pitch函数

