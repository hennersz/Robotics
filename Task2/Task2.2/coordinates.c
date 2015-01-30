#include <stdio.h>
#include <math.h>

float findAngle(int *leftEncoder, int *rightEncoder)
{
	float ratio = 2.333;
	return ((float)(*leftEncoder - *rightEncoder) * ratio) % 360;
}


//Do not forget to initialize encoders!!!!
float distanceTravelled(int *leftEncoder, int *rightEncoder, float *x, float *y)
{
	int previousLeft = *leftEncoder;
	int previousRight = *rightEncoder;
	get_motor_encoders(leftEncoder, rightEncoder);
	float angle = findAngle(leftEncoder, rightEncoder);
	float distance = ((float)(*leftEncoder - previousLeft) + (*rightEncoder - previousLeft)) / 2
	*x +=distance * (sin(angle));
	*y += distance * (cos(angle));
}  


