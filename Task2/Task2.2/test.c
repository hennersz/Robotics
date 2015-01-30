#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "picomms.h"
#include "basicFunctions.h"

float ratio = 0.21;

void calculateRatio(float wheelDiam, float robotDiam)
{
	float wheelCirc = wheelDiam * M_PI;
	float robotCirc = robotDiam * M_PI;
	ratio = 1/(robotCirc/wheelCirc);
}

int findAngle(int *leftEncoder, int *rightEncoder)
{
	float temp;
	temp = (float)(*leftEncoder - *rightEncoder) * ratio;
	return (int)temp % 360;
}


void distanceTravelled(int *leftEncoder, int *rightEncoder, float *x, float *y)
{
	int previousLeft = *leftEncoder;
	int previousRight = *rightEncoder;
	get_motor_encoders(leftEncoder, rightEncoder);
	int angle = findAngle(leftEncoder, rightEncoder);
	float distance = (float)((*leftEncoder - previousLeft) + (*rightEncoder - previousRight)) / 2;
	printf("Angle = %i\tDistance = %f\n", angle, distance);
	*x += distance * (cos(angle));
	*y += distance * (sin(angle));
	printf("x = %f\t y = %f\n", *x, *y);
} 

int main()
{
	connect_to_robot();
	initialize_robot();
	//calculateRatio(96.0, 260.0);
	int leftEncoder, rightEncoder;
	float x,y;
	get_motor_encoders(&leftEncoder, &rightEncoder);
	while(1)
	{
		set_motors(10,-10);
		distanceTravelled(&leftEncoder, &rightEncoder, &x, &y);
	}
	
}
