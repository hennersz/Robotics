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

void goTo(Node *node, Mapping *mapping, int speed)
{
	float deltaX, deltaY, angle;
	float distance;
	int speedDifference, positionAngle;
	do{
		distanceTravelled(mapping);
		deltaX = node->x - mapping->x;  //distance between target and current point
		deltaY = node->y - mapping->y;
		distance = sqrt(deltaX*deltaX + deltaY*deltaY);
		printf("X=%f\tY=%f\tdeltaX=%f\tdeltaY=%f\n",mapping->x, mapping->y, deltaX, deltaY);
		angle = atan(deltaX/deltaY) * (180/M_PI);  //convert to degrees 
		positionAngle = findAngle(mapping->previousLeft, mapping->previousRight);
		printf("Angle=%f\n", angle);
		if(positionAngle > 90 || positionAngle < -90)
		{
			speedDifference = -(speed/10) * angle;  //implement PID??
		}
		else
			speedDifference = angle;
		set_motors(speed, speed + speedDifference);
	}
	while(!(distance < 20));
	printf("QUIT deltaX=%f\n", deltaX);
}

void goBack(List *list, Mapping *mapping, int speed)
{
	Node* currentNode = list->last;
	while(currentNode!=NULL)
	{
		float deltaX = currentNode->x - mapping->x;
		float deltaY = currentNode->y - mapping->y;
		printf("Current Position: X=%f\tY=%f\tangle=%i\n", mapping->x, mapping->y, findAngle(mapping->previousLeft, mapping->previousRight));
		printf("Target Position: X=%f\tY=%f\tangle=%f\n",currentNode->x, currentNode->y, atan(deltaX/deltaY));
		goTo(currentNode, mapping, speed);
		currentNode=currentNode->parent;
	}
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

	goBack(list, mapping, 50);

	return 0;
}