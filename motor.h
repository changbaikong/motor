#ifndef motor_H
#define motor_H

#include <Arduino.h>

class motor{   //   就是帮你在对象里保存 6 个值，免得你自己用全局变量到处传。private
  //的意思是：外面的人不能直接改，只能通过我提供的函数来操作。

  private:
  int _enablePin;
  int _in1Pin;
  int _in2Pin;
  int _currentSpeed;
  bool _isEnabled;
  bool _reversed;

  public:
  motor(int enablePin, int in1Pin, int in2Pin, bool reversed=false);
  ~motor();
  void begin();
  void forward(int speed);
  void backward(int speed);
  void stop();
  void brake();
  int getCurrentSpeed() const;
  bool isEnabled() const;
  void statetalk();
  void setSpeed(int speed);
};
#endif