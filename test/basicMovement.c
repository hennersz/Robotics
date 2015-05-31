#include "defines.h"

void turnByAngleDegree(double angle)
{
  int leftEncoder, rightEncoder;
  get_motor_encoders(&leftEncoder, &rightEncoder);
  double spinCircumference = ROBOT_WIDTH * M_PI;
  double distanceToCover = spinCircumference * (angle/360.00);
  double tickDifference = (360*distanceToCover)/(CIRCUMFERENCE);
  int targetLeftEncoder = leftEncoder + tickDifference;
  int speed;
  while(abs(targetLeftEncoder - leftEncoder) > 1)
  {
    speed = (targetLeftEncoder - leftEncoder);
    if(speed > 20)
      speed = 20;
    if(speed < -20)
      speed = -20;
    get_motor_encoders(&leftEncoder, &rightEncoder);
    set_motors(speed, -speed);
  }
  set_motors(0,0);
}

