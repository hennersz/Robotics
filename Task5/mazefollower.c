#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "picomms.h"
#include "mapping.h"
#include "wallfollower.h"

#define STOPPINGDISTANCE 20
#define FULLSTOP 10

void stop()
{
	int frontLeft = 0, frontRight = 0;
	get_front_ir_dists(&frontLeft, &frontRight);
	while(frontRight>FULLSTOP)
	{
		set_motors(frontRight, frontRight);
	}
	set_motors(0,0);
}

void setMotors(int finalValue, int speed)
{
	if(finalValue>0)
	{
		set_motors(speed, speed + finalValue);
	}
	else
	{
		set_motors(speed + finalValue, speed);
	}
}

void phase1(int speed, Mapping* mapping)
{
	printf("started phase 1\n");
	int frontLeft, frontRight, finalValue, previousFront;
	int leftBumper,rightBumper;
	int total = 0;
	double xDifference, yDifference, distance;
	get_front_ir_dists(&frontLeft, &frontRight);
	set_ir_angle(1, -45);
	while((mapping->x>=0)&&(mapping->y>=0)&&(frontRight>STOPPINGDISTANCE))
	{
		get_front_ir_dists(&frontLeft,&frontRight);
		distanceTravelled(mapping);
		finalValue = calculateMotorValue(&frontLeft,&previousFront, &total, speed);
		setMotors(finalValue, speed);
	}
	stop();
}

int main()
{

	Mapping *mapping = malloc(sizeof(Mapping));
	initialiseMapping(mapping);

	connect_to_robot();
	initialize_robot();
	set_origin();

	phase1(50,mapping);

	return 0;
}
