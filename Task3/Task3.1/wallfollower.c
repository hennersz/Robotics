#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "picomms.h"
#include "mapping.h"

#define TARGETDISTANCE 36
#define MAXSPEED 127
#define STOPPINGDISTANCE 13
float ratio;

int proportional(int *frontLeft)//calculate proportional value of how far the robot is from the wall
{
	if(*frontLeft > 50)
		return 5;
	else
		return *frontLeft - TARGETDISTANCE;
}

int differential(int *frontLeft, int *frontRight) //Rate of change of distance from the wall between 2 readings
{
	int previousFrontLeft = *frontLeft;
	get_front_ir_dists(frontLeft, frontRight);
	if(*frontLeft > 50)
		return 0;
	else
		return *frontLeft - previousFrontLeft;
}

void stopped(int *lBump, int *rBump)
{								//checks to see if the robot has chrashed
	check_bumpers(lBump, rBump);
	if(*lBump == 1 || *rBump == 1)
	{
		int i = 0;
		while (i < 200)
		{
			set_motors(-30, -30);
			i++;
		}
	}
}



int calculateMotorValue(int *frontLeft, int *frontRight, int *integralValue, int speed)
{																					
	int differentialValue = differential(frontLeft, frontRight);
	int proportionalValue = proportional(frontLeft);
	*integralValue +=proportionalValue;
	if (*integralValue > 100 && *integralValue < -100)   //Limit the impact of integral value
		*integralValue = 0;
	float finalValue = proportionalValue * (speed/20) + differentialValue * 30 + *integralValue * 0.1;
	finalValue*=0.5;

	if(finalValue > MAXSPEED)		//filters high or low speeds
		finalValue = MAXSPEED;
	else if(finalValue < -MAXSPEED)
		finalValue = - MAXSPEED;

	//int finalSpeed = speed + finalValue;

	return finalValue;
}

void checkWalls(int frontLeft, int frontRight, int speed, int finalValue)
{
	printf("finalValue = %i\n", finalValue);

	if(frontRight < 30)
		set_motors(frontRight, frontRight);
	else if(finalValue < -2)
		set_motors(speed - finalValue, speed);
	else
		set_motors(speed, finalValue + speed);
}

void wallFollower(int speed)
{
	int frontleft, frontright, finalValue;
	int leftBumper,rightBumper;
	int previousLeft = 0, previousRight = 0;
	double previousAngle = 0;
	float x= 0, y = 0;
	int total = 0;
	set_ir_angle(1, -45);
	calculateRatio();
	while(1)
	{
		distanceTravelled(&previousAngle, &x, &y, &previousLeft, &previousRight);
		finalValue = calculateMotorValue(&frontleft, &frontright, &total, speed);
		stopped(&leftBumper, &rightBumper);
		checkWalls(frontleft, frontright, speed, finalValue);
		log_trail();
		if(frontright <= STOPPINGDISTANCE)
		{
			set_motors(0, 0);
			break;
		}

	}
}