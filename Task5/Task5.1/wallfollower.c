#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "picomms.h"
#include "mapping.h"

#define TARGETDISTANCE 25
#define MAXSPEED 127
#define STOPPINGDISTANCE 50

int proportional(int *front, int side)//calculate proportional value of how far the robot is from the wall
{
	if(*front > 50 && side > 30)
		return 10;
	else if (*front > 50)
		return side - TARGETDISTANCE;
	else
		return (*front + side)/2 - TARGETDISTANCE;
}

int differential(int *front,int* previousFront, int side, int* previousSide) //Rate of change of distance from the wall between 2 readings
{	
	if(*front > 50 && side > 30)
		return 0;
	int value = ((*front - *previousFront)+(side-*previousSide))/2;
	*previousFront = *front;
	*previousSide = side;
	return value;

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



int calculateMotorValue(int *front,int *previousFront,int side, int* previousSide, int *integralValue, int speed)
{																	
	int differentialValue = differential(front, previousFront, side, previousSide);
	int proportionalValue = proportional(front, side);
	*integralValue +=proportionalValue;
	if (*integralValue > 100 && *integralValue < -100)   //Limit the impact of integral value
		*integralValue = 0;
	double finalValue = proportionalValue * (speed/10) + differentialValue * 20 + *integralValue * 0.1;

	if(finalValue > MAXSPEED)		//filters high or low speeds
		finalValue = MAXSPEED;
	else if(finalValue < -MAXSPEED)
		finalValue = - MAXSPEED;

	return finalValue;
}

void checkWalls(double distance, int speed, int finalValue, bool leftCloser)
{
	if(distance < 200) {
		set_motors(distance/10, distance/10);
		printf("Slowing down\n");
	}
	if (finalValue > 0 && leftCloser)
		set_motors(speed, finalValue + speed);
	else if(leftCloser)
		set_motors(speed - finalValue, speed);
	else if(finalValue > 0)
		set_motors(speed + finalValue, speed);
	else
		set_motors(speed, speed - finalValue);
}