#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "picomms.h"
#define TARGETDISTANCE 25
#define MAXSPEED 127
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

double toRadians(double angle)
{
	return (double)(angle * (M_PI/180));
}


void distanceTravelled(int *leftEncoder, int *rightEncoder, float *x, float *y)
{
	int previousLeft = *leftEncoder;
	int previousRight = *rightEncoder;
	get_motor_encoders(leftEncoder, rightEncoder);
	int angle = findAngle(leftEncoder, rightEncoder);
	float distance = (float)((*leftEncoder - previousLeft) + (*rightEncoder - previousRight)) / 2;
	//printf("Angle = %i\tDistance = %f\n", angle, distance);
	double radians = toRadians((double)angle);
	//printf("Radians = %f\n", radians);
	*y += (distance * (cos(radians))) / 12; // 12 clicks = 1 cm
	*x += (distance * (sin(radians))) / 12; 
	printf("x = %f\t y = %f\n", *x, *y);
} 
int proportional(int *frontLeft, int *backLeft)		//calculate proportional value of how far the robot is from the wall
{
	if(*frontLeft > 50 && *backLeft > 35)          //uses both front and side sensors but filters out high values
		return 5;
	else if (*backLeft > 35)
		return *frontLeft - TARGETDISTANCE;
	else if(*frontLeft > 50)
		return *backLeft - (TARGETDISTANCE - TARGETDISTANCE/4);
	else
		return (*frontLeft + *backLeft)/2 - TARGETDISTANCE;
}

int differential(int *frontLeft, int *frontRight, int *backLeft, int *backRight)  //Rate of change of distance from the wall between 2 readings
{
	int previousFrontLeft = *frontLeft;
	int previousBackLeft = *backLeft;
	get_front_ir_dists(frontLeft, frontRight);
	get_side_ir_dists(backLeft, backRight);
	int currentAverage, previousAverage;
	if(*frontLeft > 50 && *backLeft > 35)	//filter out large values
	{
		currentAverage = 0;
		previousAverage = 0;
	}	
	else if (*backLeft > 35)
	{
		currentAverage = *frontLeft;
		previousAverage = previousFrontLeft;
	}
	else if(*frontLeft > 50)
	{
		currentAverage = *backLeft;
		previousAverage = previousBackLeft;
	}
	else
	{
		currentAverage = (*frontLeft + *backLeft)/2;
		previousAverage = (previousFrontLeft + previousBackLeft)/2;
	}
	return currentAverage - previousAverage;
}

int integral(int total, int proportionalValue)  //Integral value is sum of all the previous values
{
	total += proportionalValue;
	return total;
}

void stopped(int *lBump, int *rBump)
{												//checks to see if the robot has chrashed
	check_bumpers(lBump, rBump);
	if(*lBump == 1 || *rBump == 1)
	{
		int i = 0;
		while (i < 200)
		{
			set_motors(-10, -10);
			i++;
		}
	}
}



int calculateMotorValue(int *frontLeft, int *frontRight, int *backLeft, int *backRight, int integralValue, int speed)
{																					
	int differentialValue = differential(frontLeft, frontRight, backLeft, backRight);
	int proportionalValue = proportional(frontLeft, backLeft);
	integralValue = integral(integralValue, proportionalValue);
	float finalValue = proportionalValue * 2.3 + differentialValue * 30 + integralValue * 0.05;
	int finalLeftSpeed = speed - finalValue;
	int finalRightSpeed = speed + finalValue;

	if(finalLeftSpeed < -MAXSPEED || finalRightSpeed > MAXSPEED)		//filters high or low speeds
		set_motors(-MAXSPEED, MAXSPEED);
	else if(finalLeftSpeed > MAXSPEED|| finalRightSpeed < -MAXSPEED)
		set_motors(MAXSPEED, -MAXSPEED);
	else
		set_motors(finalLeftSpeed, finalRightSpeed);

	if (integralValue < 100 && integralValue > -100)   //Limit the impact of integral value
		return integralValue;
	else 
		return 0;
}

void checkWalls(int *frontLeft, int *frontRight, int *sideLeft, int *sideRight, float *x, float *y)	//Still not sure about this one!!
{
	if((*frontLeft < 38 && *frontRight < 38) || *frontRight < 15)  //obstacle in front
	{
		int leftEncoder, rightEncoder;
		get_motor_encoders(&leftEncoder, &rightEncoder);
		if(*sideLeft < *sideRight)
			while((*frontLeft < 38 && *frontRight < 38) || *frontRight < 15)
            {
                set_motors(10,-10);
                get_front_ir_dists(frontLeft, frontRight);
                get_side_ir_dists(sideLeft, sideRight);
                distanceTravelled(&leftEncoder, &rightEncoder, x, y);
            }
		else
            while((*frontLeft < 38 && *frontRight < 38) || *frontRight < 15)
            {
                set_motors(-10,10);
                get_front_ir_dists(frontLeft, frontRight);
                get_side_ir_dists(sideLeft, sideRight);
                distanceTravelled(&leftEncoder, &rightEncoder, x, y);
            }
	}
}

void wallFollower(int speed)
{
	int frontleft, frontright; 
	int sideleft, sideright; 
	int leftBumper,rightBumper;
	int leftEncoder, rightEncoder;
	float x= 0, y = 0;
	int total = 0;
	set_ir_angle(1, -45);
	get_motor_encoders(&leftEncoder, &rightEncoder);
	while(1)
	{
		total = calculateMotorValue(&frontleft, &frontright, &sideleft, &sideright, total, speed);
		checkWalls(&frontleft, &frontright,&sideleft, &sideright, &x, &y);
		stopped(&leftBumper, &rightBumper);
		distanceTravelled(&leftEncoder, &rightEncoder, &x, &y);
	}
}

int main()
{
	connect_to_robot();
	initialize_robot();
	//calculateRatio(96.0, 260.0);
	int leftEncoder, rightEncoder;
	float x = 0,y = 0;
	get_motor_encoders(&leftEncoder, &rightEncoder);
	wallFollower(60);
	distanceTravelled(&leftEncoder, &rightEncoder, &x, &y);
}