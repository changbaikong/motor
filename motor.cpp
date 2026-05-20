#include "Arduino.h"
#include "motor.h"


motor::motor(int enablePin, int in1Pin, int in2Pin, bool reversed){
  _enablePin = enablePin;
  _in1Pin = in1Pin;
  _in2Pin = in2Pin;
  _currentSpeed = 0;
  _isEnabled = false;
  _reversed = reversed;

}
motor::~motor(){
  stop();
  Serial.print("motor on pins(");
  Serial.print(_enablePin);
  Serial.print(",");
  Serial.print(_in1Pin);
  Serial.print(",");
  Serial.print(_in2Pin);
  Serial.println(") has stopped and destroyed");
}

void motor::begin(){
  pinMode(_enablePin,OUTPUT);
  pinMode(_in1Pin,OUTPUT);
  pinMode(_in2Pin,OUTPUT);
  stop();
  Serial.println("motor is begin");

}
void motor::forward(int speed){
  speed = constrain(speed,0,255);
  digitalWrite(_in1Pin, _reversed?LOW:HIGH);
  digitalWrite(_in2Pin, _reversed?HIGH:LOW);
  analogWrite(_enablePin,speed);
  _currentSpeed = speed;
  _isEnabled = (speed > 0);
}
void motor::backward(int speed){
  speed = constrain(speed, 0, 255);
  digitalWrite(_in1Pin, _reversed?HIGH:LOW);
  digitalWrite(_in2Pin, _reversed?LOW:HIGH);
  analogWrite(_enablePin, speed);
  _currentSpeed = -speed;
  _isEnabled = (speed > 0);
}
void motor::stop(){
  analogWrite(_enablePin,0);
  digitalWrite(_in1Pin,LOW);
  digitalWrite(_in2Pin,LOW);
  _currentSpeed = 0;
  _isEnabled = false;
}
void motor::brake(){
  digitalWrite(_in1Pin,HIGH);
  digitalWrite(_in2Pin,HIGH);
  analogWrite(_enablePin,255);
  delay(50);
  stop();
  Serial.println("Motor braked.");
}
void motor::statetalk(){
    Serial.print("motor(EN=");
    Serial.print(_enablePin);
    Serial.print(") speed: ");
    Serial.print(getCurrentSpeed());
    Serial.print(" | enabled: ");
    Serial.print(isEnabled() ? "YES" : "NO");
    Serial.println();
}
int motor::getCurrentSpeed() const {
  return _currentSpeed;
}
bool motor::isEnabled() const {
  return _isEnabled;
}

void motor::setSpeed(int speed){
  if (speed > 0){
    forward(speed);
  } else if (speed < 0){
    backward(-speed);
  } else {
    stop();
  }
}