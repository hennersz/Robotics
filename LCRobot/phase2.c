#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include "picomms.h"
#include "turning.h"
#include "linkedList.h"

#define LIMIT 50
#define MINIMUM_DISTANCE 350
#define MAX 127

double findTargetAngle1(Mapping *mapping, Point *point)
{
	double deltaX = point->x - mapping->x;
	double deltaY = point->y - mapping->y;
	double targetAngle = atan2(deltaX, deltaY);
	return targetAngle;
}

int tooClose1(Point *point, Mapping *mapping, double minDistance)
{
	double xDifference = point->x - mapping->x;
	double yDifference = point->y - mapping->y;
	double distance = sqrt(xDifference*xDifference + yDifference*yDifference);
	if(distance<minDistance)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int calculateSpeedOffset1(Mapping *mapping, Point *point, int orientation, int speed)
{
	double targetAngle = findTargetAngle1(mapping, point);
	double robotAngle = mapping->previousAngle;
	if(robotAngle > M_PI)
	{
		robotAngle -= 2*M_PI;
	}
	else if (robotAngle < -M_PI) 
	{
		robotAngle +=2*M_PI;
	}
	if(targetAngle < 0 || targetAngle > M_PI/2)
		return 0;

	double angleDifference = robotAngle - targetAngle;
	
	if(angleDifference > M_PI)
		angleDifference -= 2*M_PI;
	else if(angleDifference < -M_PI)
		angleDifference += 2*M_PI;

	int speedOffset = (int)toDegrees(angleDifference);
	int speedFactor = round((double)speed / 30.0);
	speedOffset *= speedFactor;
	//if(speedOffset > LIMIT)
	//	speedOffset = LIMIT;
	//else if(speedOffset < -LIMIT)
	//	speedOffset = -LIMIT;

	return speedOffset;
}

void goTo1(Mapping *mapping, Point *point, int speed, int orientation)
{
	distanceTravelled(mapping);
	double speedOffset = (double)calculateSpeedOffset1(mapping, point, orientation, speed);
	speedOffset = speedOffset / 2;
	int leftSpeed = speed - (int)speedOffset;
	int rightSpeed = speed + (int)speedOffset;
	if (leftSpeed < -MAX)
		leftSpeed = -MAX;
	else if(leftSpeed > MAX)
		leftSpeed = MAX;
	if(rightSpeed < -MAX)
		rightSpeed = -MAX;
	else if(rightSpeed > MAX)
		rightSpeed = MAX;
	//printf("rightSpeed = %i\n", rightSpeed);
	set_motors(leftSpeed, rightSpeed);
	//set_motors1(leftSpeed, rightSpeed);
}

void findNextPoint1(Mapping *mapping, Point *point, Point *targetPoint)
{
	double deltaX = targetPoint->x - mapping->x;
	double deltaY = targetPoint->y - mapping->y;
	double angle = atan2(deltaY, deltaX);
	point->x = mapping->x + (MINIMUM_DISTANCE * cos(angle));
	point->y = mapping->y + (MINIMUM_DISTANCE * sin(angle));
}

void goToPoint1(Mapping *mapping, Point *targetPoint, Point *tempPoint, int speed, int orientation)
{
	findNextPoint1(mapping, tempPoint, targetPoint); 
	goTo1(mapping, tempPoint, speed, orientation);
}

void preparePointPhase2(Mapping *mapping, Point *targetPoint, int speed, int orientation)
{
	Point *tempPoint = malloc(sizeof(Point));
	initialisePoint(tempPoint);
	printf("targetPoint: x = %f, y = %f\n", targetPoint->x, targetPoint->y);

	while(!tooClose1(targetPoint, mapping, MINIMUM_DISTANCE))
	{
		goToPoint1(mapping, targetPoint, tempPoint, speed, orientation);
	}	
	free(tempPoint);
}