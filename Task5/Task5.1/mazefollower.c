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
#define TARGETDISTANCE 45

void stop()
{
	printf("stopped\n");
	int frontLeft = 0, frontRight = 0;
	get_front_ir_dists(&frontLeft, &frontRight);
	while(frontRight>FULLSTOP)
	{
		get_front_ir_dists(&frontLeft, &frontRight);
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
		set_motors(speed - finalValue, speed);
	}
}

void rightAngleTurn(int* frontLeft, int* frontRight, int speed)
{
	while(*frontRight<TARGETDISTANCE)
	{
		get_front_ir_dists(frontLeft, frontRight);
		set_motors(speed + 25, speed);
	}
}

void phase1(int speed, Mapping* mapping)
{
	printf("started phase 1\n");
	int frontLeft = 0, frontRight = 0, finalValue, previousFront = 0;
	int leftBumper,rightBumper;
	int total = 0;
	get_front_ir_dists(&frontLeft, &frontRight);
	previousFront = frontLeft;
	set_ir_angle(1, -45);
	printf("x:%f, y%f\n",mapping->x,mapping->y);
	while(!((mapping->x<0)&&(mapping->y<0)&&(frontRight<STOPPINGDISTANCE)))
	{
		get_front_ir_dists(&frontLeft,&frontRight);
		distanceTravelled(mapping);
		stopped(&leftBumper,&rightBumper);
		finalValue = calculateMotorValue(&frontLeft,&previousFront, &total, speed);
		printf("finalValue %i\n",finalValue);
		rightAngleTurn(&frontLeft, &frontRight, speed);
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

	phase1(30,mapping);

	return 0;
}
