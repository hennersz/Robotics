#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include "picomms.h"
#include "turning.h"

#define MAXSPEED 50

double distance(Mapping *mapping)
{
	double distance = sqrt(mapping->x*mapping->x + mapping->y*mapping->y);
	return distance;
}
void oneMeter(Mapping *mapping)
{
	int speed, difference;
	while(mapping->y < 1000)
	{
		distanceTravelled(mapping);
		difference = 1000-distance(mapping);
		if(difference > MAXSPEED)
		{
			speed = MAXSPEED;
		}
		else
		{
			speed = (int)difference;
		}
		set_motors(speed, speed);
	}
	set_motors(0,0);
}

void square(Mapping *mapping)
{
	int i;
	for(i = 0; i < 4; i++)
	{
		oneMeter(mapping);
		turn(mapping,'R', 90, 50);
	}
}
void turnTest(Mapping *mapping, int angle, int speed)
{
	turn(mapping, 'L', angle, speed);
}

int main()
{
	connect_to_robot();
	initialize_robot();

	int speed = 50;
	int angle = 1800;
	int frontLeft, frontRight, sideLeft, sideRight;
	printf("width = %i, wheel diameter = %i\n", WIDTH, WHEELDIAM);

	Mapping *mapping = malloc(sizeof(Mapping));
	initialiseMapping(mapping);

	set_ir_angle(0, -45);
	set_ir_angle(1, 45);
	square(mapping);
	/*
	while(1)
	{
		get_side_ir_dists(&sideLeft, &sideRight);
		get_front_ir_dists(&frontLeft, &frontRight);
		printf("Frontleft: %i, Sideleft: %i\tFrontRight: %i, sideRight %i\n", frontLeft, sideLeft, frontRight, sideRight);
	}
	*/

	//oneMeter(mapping);
	//usleep(10000000);
	//turnTest(mapping, angle, speed);
	return 0;
}