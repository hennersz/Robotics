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

#define LIMIT 70

float findTargetAngle(Mapping *mapping, Node *node, float deltaX, float deltaY)
{
	float angle = atan(deltaX/deltaY) * (180/M_PI);  //The angle between the 2 points in degrees
	int angleRobot = abs(findAngle(mapping->previousLeft, mapping->previousRight));

	if((node->x < mapping->x) && (node->y > mapping->y)) //2st quadrant	
		angle = angle + angleRobot;
	else if((node->x < mapping->x) && (node->y < mapping->y)) //3rd quadrant
		angle = angleRobot + angle - 90;
	else if((node->x > mapping->x) && (node->y < mapping->y))  //4th quadrant
		angle = angleRobot + angle - 180;

	return angle;
	//return atan2(node->x, node->y) - atan2(mapping->x, mapping->y);
}

 
void goTo2(Node *node, Mapping *mapping, int speed)
{
	float deltaX = node->x - mapping->x;
	float deltaY = node->y - mapping->y;
	float targetDist = sqrt(deltaX*deltaX + deltaY*deltaY);
	float distance1, previousX, previousY, angle;
	float previousAngle;
	float distance = 0;
	int speedDifference, proportional, differential, integral;

	while(distance < targetDist)
	{
		previousX = mapping->x;
		previousY = mapping->y;
		previousAngle = angle;
		distanceTravelled(mapping);
		set_point(mapping->x/10, mapping->y/10);
		deltaX = mapping->x - previousX;   //distance between current point and previous point
		deltaY = mapping->y - previousY;
		distance += sqrt(deltaX*deltaX + deltaY*deltaY); //sees how far it has travelled

		deltaX = node->x - mapping->x;  //distance between target and current point
		deltaY = node->y - mapping->y;
		distance1 = sqrt(deltaX*deltaX + deltaY*deltaY);
		angle = findTargetAngle(mapping, node, deltaX, deltaY); //The angle between the 2 points in degrees
		proportional = angle;// * distance1;
		differential = angle - previousAngle;
		integral += proportional;
		if (integral > 100 && integral < -100)   //Limit the impact of integral value
			integral = 0;

		speedDifference = proportional * ((float)speed / 75.0) + 20 * differential;// + 0.01 * integral; //1800
		
		if (speedDifference > LIMIT)
			speedDifference = LIMIT;
		else if(speedDifference < -LIMIT)	
			speedDifference = -LIMIT;

		//printf("deltaX=%f\tdeltaY=%f\tspeedDifference = %i\tdistance = %f\tangle =%f\n",deltaX, deltaY, speedDifference, distance, angle);

		if(speedDifference > 0)
			set_motors(speed + speedDifference, speed);
		else
			set_motors(speed, speed - speedDifference);
	}
}


void goBack(List *list, Mapping *mapping, int speed)
{
	Node* currentNode = list->last;
	distanceTravelled(mapping);
	while(currentNode!=NULL)
	{
		float deltaX = currentNode->x - mapping->x;
		float deltaY = currentNode->y - mapping->y;
		printf("Current Position: X=%f\tY=%f\tangle=%i\n", mapping->x, mapping->y, findAngle(mapping->previousLeft, mapping->previousRight));
		printf("Target Position: X=%f\tY=%f\tangle=%f\n",currentNode->x, currentNode->y, atan(deltaX/deltaY));
		if(sqrt(deltaX * deltaX + deltaY * deltaY) > 10)
			goTo2(currentNode, mapping, speed);
		currentNode=currentNode->parent;
	}
}

void plotPoints(List *list)
{
	Node* currentNode = list->last;
	while(currentNode!=NULL)
	{
		set_point(currentNode->x/10, currentNode->y/10);
		printf("PLOTTED POINTS: %f\t%f\n", currentNode->x/10,currentNode->y/10);
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
	//plotPoints(list);

	goBack(list, mapping, 30);

	return 0;
}