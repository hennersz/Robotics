#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include "picomms.h"
#include "turning.h"

#define MAXSPEED 127

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
			speed = 127;
		}
		else
		{
			speed = (int)difference;
		}
		set_motors(speed, speed);
	}
	set_motors(0,0);
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

	printf("width = %i, wheel diameter = %i\n", WIDTH, WHEELDIAM);

	Mapping *mapping = malloc(sizeof(Mapping));
	initialiseMapping(mapping);

	oneMeter(mapping);
	//usleep(10000000);
	//turnTest(mapping, angle, speed);
	return 0;
}