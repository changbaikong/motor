#include "imu.h"
#include "oled.h"

// ==================== 硬件 ====================
static MPU6050 mpu;
static uint8_t devStatus;
static uint8_t fifoBuffer[64];
static Quaternion q;

// 敏感度：dmpInitialize 后固定
static const float ACCEL_SCALE = 16384.0;   // ±2g
static const float GYRO_SCALE  = 16.4;      // ±2000°/s

// 传感器缓存（readSensors 填充）
static float ax_g, ay_g, az_g;     // 加速度（g）
static float gx_dps, gy_dps;       // 陀螺仪（°/s）
int16_t gyro_z_raw = 0;            // 陀螺仪 Z 轴原始值

// ==================== Pitch / Roll 互补滤波 ====================
float pitch_cf = 0.0;
float roll_cf  = 0.0;
static float smoothDt = 0.0;
static unsigned long prevTime_cf = 0;
static const float ALPHA = 0.96;

// ==================== Yaw DMP + 死区 + 漂移补偿 ====================
float yaw       = 0.0;
static float yawRaw       = 0.0;
static float yawDriftRate = 0.0;
static float yawFrozen    = 0.0;
static unsigned long lastYawUs = 0;

bool dmpReady       = false;
static bool yawCalibrated  = false;

static const int16_t GYRO_Z_DEADZONE       = 30;
static const float   DRIFT_CALIBRATION_SEC = 10.0;


// ==================== 静止检测 ====================

static bool waitForStill() {
  const float STILL_THRESHOLD = 0.05;
  int stillCount = 0;
  int16_t ax, ay, az, last_ax, last_ay, last_az;

  mpu.getAcceleration(&last_ax, &last_ay, &last_az);
  Serial.println("等待传感器静止...");

  while (stillCount < 50) {
    mpu.getAcceleration(&ax, &ay, &az);
    float diff = sqrt(sq((float)(ax - last_ax)) +
                      sq((float)(ay - last_ay)) +
                      sq((float)(az - last_az))) / ACCEL_SCALE;
    if (diff < STILL_THRESHOLD) stillCount++;
    else stillCount = 0;

    last_ax = ax; last_ay = ay; last_az = az;

    OLED.clearDisplay();
    OLED.setTextSize(1);
    OLED.setCursor(0, 0);
    OLED.print(F("Keep Still"));
    OLED.setCursor(0, 16);
    OLED.print(F("Calibrating..."));
    int barWidth = map(stillCount, 0, 50, 0, 128);
    OLED.fillRect(0, 40, barWidth, 12, SSD1306_WHITE);
    OLED.display();
    delay(20);
  }
  Serial.println("传感器已静止");
  return true;
}


// ==================== 初始化 ====================

void initIMU() {
  // Wire 已在 initOLED() 中初始化（SDA=20, SCL=21），此处不重复调用
  Wire.setClock(400000);

  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 连接失败，请检查接线！");
    while (1) delay(10);
  }

  waitForStill();

  devStatus = mpu.dmpInitialize();
  if (devStatus != 0) {
    Serial.print("DMP 初始化失败，错误码: ");
    Serial.println(devStatus);
    while (1) delay(10);
  }

  OLED.clearDisplay();
  OLED.setCursor(0, 0);
  OLED.print(F("Keep Still!"));
  OLED.setCursor(0, 20);
  OLED.print(F("DMP Calibrating..."));
  OLED.display();

  mpu.CalibrateGyro(6);
  mpu.PrintActiveOffsets();
  mpu.setDMPEnabled(true);
  dmpReady = true;

  // 互补滤波初始角度
  {
    int16_t iax, iay, iaz;
    mpu.getAcceleration(&iax, &iay, &iaz);
    ax_g = iax / ACCEL_SCALE;
    ay_g = iay / ACCEL_SCALE;
    az_g = iaz / ACCEL_SCALE;
    roll_cf  = atan2(ay_g, az_g) * 180.0 / M_PI;
    pitch_cf = atan2(-ax_g, sqrt(ay_g*ay_g + az_g*az_g)) * 180.0 / M_PI;
  }
  prevTime_cf = micros();

  // 标定 Yaw 漂移速率
  Serial.println("标定 Yaw 漂移速率...（请保持传感器静止）");

  unsigned long calibStart = millis();
  float yawStart = 0.0, yawEnd = 0.0;
  bool  firstRead = true;

  while (millis() - calibStart < DRIFT_CALIBRATION_SEC * 1000) {
    if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) {
      mpu.dmpGetQuaternion(&q, fifoBuffer);
      float cyaw = atan2(2.0*(q.w*q.z + q.x*q.y),
                         1.0 - 2.0*(q.y*q.y + q.z*q.z)) * 180.0 / M_PI;
      if (firstRead) { yawStart = cyaw; firstRead = false; }
      yawEnd = cyaw;

      // 每帧完整重画（clearDisplay 会清掉上一帧的文字）
      OLED.clearDisplay();
      OLED.setCursor(0, 0);
      OLED.print(F("Keep Still!"));
      OLED.setCursor(0, 20);
      OLED.print(F("Yaw calib..."));
      OLED.setCursor(0, 40);
      int pct = (millis() - calibStart) * 100 / (DRIFT_CALIBRATION_SEC * 1000);
      OLED.print(pct); OLED.print('%');
      OLED.display();
    }
  }

  yawDriftRate  = (yawEnd - yawStart) / DRIFT_CALIBRATION_SEC;
  yawCalibrated = true;

  // Serial.print("Yaw 漂移速率: ");
  // Serial.print(yawDriftRate * 3600.0, 3);
  // Serial.print(" °/h  (");
  // Serial.print(yawDriftRate, 5);
  // Serial.println(" °/s)");

  yawFrozen = yawEnd;
  lastYawUs = micros();

  Serial.println("初始化完成！Yaw(Y)  Pitch(P)  Roll(R)");
}


// ==================== 传感器读取 ====================

bool readSensors() {
  // DMP 四元数
  if (!mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) return false;
  mpu.dmpGetQuaternion(&q, fifoBuffer);

  float norm = sqrt(q.w*q.w + q.x*q.x + q.y*q.y + q.z*q.z);
  if (norm > 0.0f) { q.w /= norm; q.x /= norm; q.y /= norm; q.z /= norm; }

  // 原始加速度 + 陀螺仪
  int16_t rax, ray, raz, rgx, rgy;
  mpu.getAcceleration(&rax, &ray, &raz);
  mpu.getRotation(&rgx, &rgy, &gyro_z_raw);

  ax_g   = rax / ACCEL_SCALE;
  ay_g   = ray / ACCEL_SCALE;
  az_g   = raz / ACCEL_SCALE;
  gx_dps = rgx / GYRO_SCALE;
  gy_dps = rgy / GYRO_SCALE;

  return true;
}


// ==================== 姿态更新 ====================

void updatePose() {
  unsigned long nowUs = micros();
  float dt = (nowUs - prevTime_cf) / 1000000.0f;
  prevTime_cf = nowUs;
  if (dt > 0.01f) dt = 0.01f;

  if (smoothDt == 0.0f) smoothDt = dt;
  smoothDt = 0.9f * smoothDt + 0.1f * dt;

  // ---- Pitch ----
  float accPitch  = atan2(-ax_g, sqrt(ay_g*ay_g + az_g*az_g)) * 180.0 / M_PI;
  float gyroPitch = pitch_cf + gy_dps * smoothDt;

  float diff = fabs(accPitch - gyroPitch);
  float alpha;
  if (diff > 10.0f)      alpha = 0.50f;
  else if (diff > 3.0f)  alpha = 0.80f;
  else                   alpha = ALPHA;

  pitch_cf = alpha * gyroPitch + (1.0f - alpha) * accPitch;

  // ---- Roll ----
  float accRoll  = atan2(ay_g, az_g) * 180.0 / M_PI;
  float gyroRoll = roll_cf + gx_dps * smoothDt;

  diff = fabs(accRoll - gyroRoll);
  if (diff > 10.0f)      alpha = 0.50f;
  else if (diff > 3.0f)  alpha = 0.80f;
  else                   alpha = ALPHA;

  roll_cf = alpha * gyroRoll + (1.0f - alpha) * accRoll;
}

void updateYaw() {
  yawRaw = atan2(2.0*(q.w*q.z + q.x*q.y),
                 1.0 - 2.0*(q.y*q.y + q.z*q.z)) * 180.0 / M_PI;

  unsigned long nowUs = micros();
  float dt = (nowUs - lastYawUs) / 1000000.0f;
  lastYawUs = nowUs;
  
  // 死区冻结关闭暂时
  // if (abs(gyro_z_raw) < GYRO_Z_DEADZONE) return;

  // yawFrozen = yawRaw;

  if (yawCalibrated) {
    static float driftAccum = 0.0;
    driftAccum += yawDriftRate * dt;
    yaw = yawRaw - driftAccum;
  } else {
    yaw = yawRaw;
  }
}
