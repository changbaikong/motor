#include "HardwareSerial.h"
#include "oled.h"

#define I2C_SDA 20
#define I2C_SCL 21

Adafruit_SSD1306 OLED(128, 64, &Wire, -1);

void initOLED() {
#ifdef ARDUINO_ARCH_ESP32
  Wire.begin(I2C_SDA, I2C_SCL);
#else
  Wire.begin();
#endif
  if (!OLED.begin(SSD1306_SWITCHCAPVCC, 0x3c)) {
    Serial.println("OLED not found");
    while (1);
  }
  OLED.setTextColor(1);
  OLED.clearDisplay();
}

// void updateDisplay() {
//   static unsigned long lastUpdate = 0;
//   if (millis() - lastUpdate > 40) {
//     lastUpdate = millis();
//     // showAngleDisplay();
//   }
// }

// void showAngleDisplay() {
//   int pitch = int(getPitchDisplay());
//   int roll = int(getRollDisplay());
//   int yaw = int(getyaw());

//   OLED.clearDisplay();
//   OLED.setTextSize(1);
//   OLED.setCursor(0, 0);

//   OLED.print("Pitch:");
//   OLED.println(pitch);
//   OLED.print("Roll:");
//   OLED.println(roll);
//   OLED.print("Yaw:");
//   OLED.println(yaw);
//   OLED.display();

//   // VOFA+ JustFloat 协议：浮点数组 + 帧尾
//   // 帧尾 {0x00, 0x00, 0x80, 0x7f} = 小端 float 正无穷(NaN)，JustFloat 用它判断帧边界
//   float vofa[3] = {(float)roll, (float)-pitch, (float)yaw};
//   Serial.write((uint8_t*)vofa, 12);
//   char tail[4] = {0x00, 0x00, 0x80, 0x7f};
//   Serial.write(tail, 4);
// }

void showStartupScreen() {
  OLED.clearDisplay();
  OLED.setCursor(20, 30);
  OLED.print("Smart Car Open");
  OLED.display();
  delay(2000);
}

void showStatusDisplay() {
  OLED.clearDisplay();
  OLED.setTextSize(2);
  OLED.setCursor(10, 20);
  OLED.print("Status OK");
  OLED.display();
  delay(1000);
}
