#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include "picomms.h"
#include "mapping.h"

#define MAXIMUM 127

double determineTargetAngle(char direction, int angle, double previousAngle)
{
	double targetAngle = toRadians(angle);
	if(direction == 'L')
		return previousAngle - targetAngle;
	else if(direction == 'R')
		return previousAngle + targetAngle;
	else
		return previousAngle;
}

int calculateSpeed(double difference)
{
	int degreesDifference = (int)toDegrees(difference);
	if(degreesDifference > MAXIMUM)
		return MAXIMUM;
	else if(degreesDifference < -MAXIMUM)
		return -MAXIMUM;
	else
		return degreesDifference;
}

void turn(Mapping *mapping, char direction, int angle, int speed)
{
	int speedDifference = 1;
	double difference;
	double targetAngle = determineTargetAngle(direction, angle, mapping->previousAngle);
	while(speedDifference != 0)
	{
		distanceTravelled(mapping);
		difference = targetAngle - mapping->previousAngle;
		speedDifference = calculateSpeed(difference);
		set_motors(speedDifference, -speedDifference);
		//set_motors1(speedDifference, -speedDifference);
	}
}