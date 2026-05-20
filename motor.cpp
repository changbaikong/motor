#include "Arduino.h"
#include "motor.h"

// class motor{
//   private:
//   int _enablePin;
//   int _in1Pin;
//   int _in2Pin;
//   int _currentSpeed;
//   bool _isEnabled;

//   public:
//   motor(int enablePin, int in1Pin, int in2Pin);
//   ~motor();
//   void begin();
//   void forward(int speed);
//   void backward(int speed);
//   void stop();
//   void brake();
//   int getCurrentSpeed() const;
//   bool isEnabled() const;

//   void setSpeed(int speed);
// };
motor::motor(int enablePin, int in1Pin, int in2Pin){
  _enablePin = enablePin;
  _in1Pin = in1Pin;
  _in2Pin = in2Pin;
  _currentSpeed = 0;
  _isEnabled = false;
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
  digitalWrite(_in1Pin, HIGH);
  digitalWrite(_in2Pin, LOW);
  analogWrite(_enablePin,speed);
  _currentSpeed = speed;
  _isEnabled = (speed > 0);
}
void motor::backward(int speed){
  speed = constrain(speed, 0, 255);
  digitalWrite(_in1Pin, LOW);
  digitalWrite(_in2Pin, HIGH);
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
void statetalk(){
    Serial.print("motorB speed: ");
    Serial.print(motorB.getCurrentSpeed());
    Serial.print(" | motorC speed: ");
    Serial.println(motorC.getCurrentSpeed());
    Serial.print("motorB enabled: ");
    Serial.print(motorB.isEnabled() ? "YES" : "NO");
    Serial.print(" | motorC enabled: ");
    Serial.println(motorC.isEnabled() ? "YES" : "NO");
    Serial.println("---------------------");
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