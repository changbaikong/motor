#ifndef motor_H
#define motor_H

#include <Arduino.h>

class motor{
  private:
  int _enablePin;
  int _in1Pin;
  int _in2Pin;
  int _currentSpeed;
  bool _isEnabled;

  public:
  motor(int enablePin, int in1Pin, int in2Pin);
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