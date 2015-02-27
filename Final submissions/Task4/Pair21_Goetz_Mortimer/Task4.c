#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "picomms.h"
#include "mapping.h"
#include "linkedList.h"
#include "wallfollower.h"
#include "basicFunctions.h"

#define LIMIT 60
#define MINIMUM_DISTANCE 100


double toDegrees(double angle)
{
	//printf("offset = %f\n", angle*(180.0/M_PI));
	return angle*(180.0/M_PI);
}

double findTargetAngle(Mapping *mapping, Node *node)
{
	double xDifference = node->x - mapping->x;
	double yDifference = node->y - mapping->y;
	double targetAngle = atan(xDifference/yDifference);
	if(node->y < mapping-> y)
	{
		targetAngle += M_PI;
	}
	else if(node->x < mapping -> x)
	{
		targetAngle += 2*M_PI;
	}
	return targetAngle;
}

int tooClose(Node *node, Mapping *mapping)
{
	double xDifference = node->x - mapping->x;
	double yDifference = node->y - mapping->y;
	double distance = sqrt(xDifference*xDifference + yDifference*yDifference);

	if(distance<MINIMUM_DISTANCE)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int calculateSpeedOffset(Mapping *mapping, Node *node)
{
	double targetAngle = findTargetAngle(mapping, node);
	double robotAngle = mapping->previousAngle;
	if(robotAngle > 2*M_PI)
	{
		robotAngle -= 2*M_PI;
	}
	else if (robotAngle <=0)
	{
		robotAngle +=2*M_PI;
	}
	int speedOffset = (int)toDegrees(robotAngle - targetAngle) * 4;  //
	
	if(speedOffset > LIMIT)
		speedOffset = LIMIT;
	else if(speedOffset < -LIMIT)
		speedOffset = -LIMIT;

	return speedOffset;
}

void goTo(Mapping *mapping, Node *node, int speed)
{
	distanceTravelled(mapping);
	double speedOffset = (double)calculateSpeedOffset(mapping, node);
	if(speedOffset > 0)
		set_motors(speed, speed + (int)speedOffset);
	else
		set_motors(speed - (int)speedOffset, speed);
}

void goBack(List *list, Mapping *mapping, int speed)
{
	Node* currentNode = list->last;
	distanceTravelled(mapping);
	while(currentNode->parent!=NULL)
	{

		if(tooClose(currentNode, mapping))
		{
			currentNode = currentNode->parent;
			set_point(currentNode->x/10, currentNode->y/10);
		}
		else
		{
			goTo(mapping, currentNode, speed);
		}
	}

	set_motors(0,0);
}

int main()
{
	List *list = malloc(sizeof(List));
	Mapping *mapping = malloc(sizeof(Mapping));
	initialiseList(list);
	initialiseMapping(mapping);

	connect_to_robot();
	initialize_robot();
	set_origin();

	wallFollower(70, list, mapping);
	turn('L', 180, 127);
	
	goBack(list, mapping, 70);

	return 0;
}