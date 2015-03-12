#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "picomms.h"
#include "mapping.h"
#include "linkedList.h"

#define TARGETDISTANCE 20
#define MAXSPEED 127
#define STOPPINGDISTANCE 13

int proportional(int *front)//calculate proportional value of how far the robot is from the wall
{
	if(*front > 50)
		return 5;
	else
		return *front - TARGETDISTANCE;
}

int differential(int *front,int* previousFront) //Rate of change of distance from the wall between 2 readings
{	
	if(*front > 50)
		return 0;
	int value = *front - *previousFront;
	*previousFront = *front;
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



int calculateMotorValue(int *front,int *previousFront, int *integralValue, int speed)
{																	
	int differentialValue = differential(front, previousFront);
	int proportionalValue = proportional(front);
	*integralValue +=proportionalValue;
	if (*integralValue > 100 && *integralValue < -100)   //Limit the impact of integral value
		*integralValue = 0;
	double finalValue = proportionalValue * (speed/10) + differentialValue * 15 + *integralValue * 0.1;

	if(finalValue > MAXSPEED)		//filters high or low speeds
		finalValue = MAXSPEED;
	else if(finalValue < -MAXSPEED)
		finalValue = - MAXSPEED;

	return finalValue;
}

void checkWalls(int forwardSensor, int speed, int finalValue, bool leftCloser)
{
	if(forwardSensor < 30)
		set_motors(forwardSensor, forwardSensor);
	if (finalValue > 0 && leftCloser)
		set_motors(speed, finalValue + speed);
	else if(leftCloser)
		set_motors(speed - finalValue, speed);
	else if(finalValue > 0)
		set_motors(speed + finalValue, speed);
	else
		set_motors(speed, speed - finalValue);
}

void wallFollower(int speed, List* list, Mapping* mapping)
{
	int frontLeft, frontRight, finalValue, previousFront;
	int leftBumper,rightBumper;
	int total = 0;
	get_front_ir_dists(&frontLeft, &frontRight);
	bool leftCloser;
	if (frontLeft<frontRight)
	{
		leftCloser = true;
		set_ir_angle(1, -45);
		previousFront = frontLeft;
	}
	else
	{
		leftCloser = false;
		set_ir_angle(0, 45);
		previousFront = frontRight;
	}
	
	while(1)
	{
		get_front_ir_dists(&frontLeft,&frontRight);
		distanceTravelled(mapping);
		if(leftCloser)
		{
			finalValue = calculateMotorValue(&frontLeft,&previousFront, &total, speed);
			checkWalls(frontRight, speed, finalValue, leftCloser);
			if(frontRight <= STOPPINGDISTANCE)
			{
				set_motors(0, 0);
				break;
			}
		}
		else
		{
			finalValue = calculateMotorValue(&frontRight,&previousFront, &total, speed);
			checkWalls(frontLeft, speed, finalValue, leftCloser);
			if(frontLeft <= STOPPINGDISTANCE)
			{
				set_motors(0, 0);
				break;
			}
		}
		stopped(&leftBumper, &rightBumper);
		
		log_trail();
		
		//printf("Added : %f\t%f\n", mapping->x/10, mapping->y/10);
		pushNode(list, mapping->x, mapping->y);
	}
}