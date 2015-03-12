#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include "picomms.h"
#include "mapping.h"
#include "linkedList.h"
#include "basicFunctions.h"

#define LIMIT 20
#define WIDTH 225
#define WHEELDIAM 96
#define MINIMUM_DISTANCE 120
#define MINDIST2 30
#define TURNINGSPEED 1
#define SENSOR_OFFSET 1


void initialisePoints(Point *points[16])
{
	int i;
	//double offset = WIDTH/2;
	for(i = 0; i < 16; i++)
	{
		initialisePoint(points[i]);
		points[i]->x = (i % 4) * 600;
		points[i]->y = (i / 4) * 600 + 400;
		points[i]->address = i;
	}
}

void initialiseWalls(bool walls[16][16])
{
	int i, j;
	for(i = 0; i < 16; i++)
	{
		for(j = 0; j < 16; j++)
			walls[i][j] = false;
	}
}

void initialiseIR() 
{
	set_ir_angle(0, -45);
	set_ir_angle(1, 45);
}

void printWalls(bool walls[16][16])
{
	int i, j;
	for(i = 0; i < 16; i++)
		printf("%i\t", i);
	printf("\n");
	for(i = 0; i < 16; i++)
	{
		printf("%i: ", i);
		for(j = 0; j < 16; j++)
			printf("%d\t", walls[i][j]);
		printf("\n");
	}
	printf("\n");
}

double findTargetAngle(Mapping *mapping, Point *point)
{
	double xDifference = point->x - mapping->x;
	double yDifference = point->y - mapping->y;
	double targetAngle = atan(xDifference/yDifference);
	if(point->y < mapping-> y)
	{
		targetAngle += M_PI;
	}
	else if(point->x < mapping -> x)
	{
		targetAngle += 2*M_PI;
	}
	return targetAngle;
}

int tooClose(Point *point, Mapping *mapping, double minDistance)
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

int calculateSpeedOffset(Mapping *mapping, Point *point)
{
	double targetAngle = findTargetAngle(mapping, point);
	double robotAngle = mapping->previousAngle;
	if(robotAngle > 2*M_PI)
	{
		robotAngle -= 2*M_PI;
	}
	else if (robotAngle < 0) 
	{
		robotAngle +=2*M_PI;
	}
	int speedOffset = (int)toDegrees(robotAngle - targetAngle);
	
	if(speedOffset > LIMIT)
		speedOffset = LIMIT;
	else if(speedOffset < -LIMIT)
		speedOffset = -LIMIT;

	return speedOffset;
}

void goTo(Mapping *mapping, Point *point, int speed)
{
	distanceTravelled(mapping);
	double speedOffset = (double)calculateSpeedOffset(mapping, point);
	if(speedOffset > 0)
		set_motors(speed, speed + (int)speedOffset);
	else
		set_motors(speed - (int)speedOffset, speed);
}

void findNextPoint(Mapping *mapping, Point *point, Point *targetPoint)
{
	double deltaX = targetPoint->x - mapping->x;
	double deltaY = targetPoint->y - mapping->y;
	double angle = atan2(deltaY, deltaX);
	point->x = mapping->x + MINIMUM_DISTANCE * cos(angle);
	point->y = mapping->y + MINIMUM_DISTANCE * sin(angle);
	set_point(point->x/10, point->y/10);
}

void goToPoint(Mapping *mapping, Point *targetPoint, Point *tempPoint, int speed)
{
	findNextPoint(mapping, tempPoint, targetPoint); 
	//set_point((int)mapping->x/10, (int)mapping->y/10);
	goTo(mapping, tempPoint, speed);
}

void preparePoint(Mapping *mapping, Point *targetPoint, int speed)
{
	Point *tempPoint = malloc(sizeof(Point));
	initialisePoint(tempPoint);
	set_point((int)targetPoint->x/10, (int)targetPoint->y/10);

	while(!tooClose(targetPoint, mapping, MINIMUM_DISTANCE))
	{
		goToPoint(mapping, targetPoint, tempPoint, speed);
	}	
	free(tempPoint);
}

void updateWalls(Mapping *mapping, int checkedWalls[3], bool walls[16][16], int address)
{
	int orientation = checkOrientation(mapping);

	if(orientation == 0)
	{
		if(checkedWalls[0] > 10)
		{
			walls[address][address - 1] = true;
			walls[address - 1][address] = true;
		}
		if(checkedWalls[1] > 10)
		{
			walls[address][address + 4] = true;
			walls[address + 4][address] = true;
		}
		if(checkedWalls[2] > 10)
		{
			walls[address][address + 1] = true;
			walls[address + 1][address] = true;
		}
	}
	else if(orientation == 1)
	{
		if(checkedWalls[0] > 10)
		{
			walls[address][address + 4] = true;
			walls[address + 4][address] = true;
		}
		if(checkedWalls[1] > 10)
		{
			walls[address][address + 1] = true;
			walls[address + 1][address] = true;
		}
		if(checkedWalls[2] > 10)
		{
			walls[address][address - 4] = true;
			walls[address - 4][address] = true;
		}
	}
	else if(orientation == 2)
	{
		if(checkedWalls[0] > 10)
		{
			walls[address][address + 1] = true;
			walls[address + 1][address] = true;
		}
		if(checkedWalls[1] > 10)
		{
			walls[address][address - 4] = true;
			walls[address - 4][address] = true;
		}
		if(checkedWalls[2] > 10)
		{
			walls[address][address - 1] = true;
			walls[address - 1][address] = true;
		}
	}
	else 
	{
		if(checkedWalls[0] > 10)
		{
			walls[address][address - 4] = true;
			walls[address - 4][address] = true;
		}
		if(checkedWalls[1] > 10)
		{
			walls[address][address - 1] = true;
			walls[address - 1][address] = true;
		}
		if(checkedWalls[2] > 10)
		{
			walls[address][address + 4] = true;
			walls[address + 4][address] = true;
		}
	}
}


void scanForWalls(Mapping *mapping, List *list, Point *targetPoint, int speed, bool walls[16][16], int address)
{
	int checkedWalls[3];
	checkedWalls[0] = 0;
	checkedWalls[1] = 0;
	checkedWalls[2] = 0;
	int frontleft, frontright, sideleft, sideright;
	int usfront;

	Point *tempPoint = malloc(sizeof(Point));
	initialisePoint(tempPoint);
	while(!tooClose(targetPoint, mapping, MINDIST2))
	{
		goToPoint(mapping, targetPoint, tempPoint, speed);
		get_front_ir_dists(&frontleft, &frontright);
		get_side_ir_dists(&sideleft, &sideright);
		usfront = get_us_dist();

		if(frontleft > 30 && sideleft > 30)
			checkedWalls[0]++;
		if(frontright > 30 && sideright > 30)
			checkedWalls[2]++;
		if(usfront > 40)
			checkedWalls[1]++;
	}
	set_motors(0, 0);
	updateWalls(mapping, checkedWalls, walls, address);
	free(tempPoint);
	targetPoint->visited = true;
	pushNode(list, targetPoint->x, targetPoint->y, address);

}

void checkTurn(bool turnedRight)
{
	int front, side;
	if(turnedRight)
	{
		do
		{
			front = get_front_ir_dist(0) - SENSOR_OFFSET; //front sensor is further from wall
			side = get_side_ir_dist(0);
			if(front > side)
				set_motors(-TURNINGSPEED, TURNINGSPEED);
			else
				set_motors(TURNINGSPEED, -TURNINGSPEED);
		}
		while(front != side);
	}
	else 
	{
		do
		{
			front = get_front_ir_dist(1) - SENSOR_OFFSET;
			side = get_side_ir_dist(1);
			if(front > side)
				set_motors(TURNINGSPEED, -TURNINGSPEED);
			else
				set_motors(-TURNINGSPEED, TURNINGSPEED);
		}
		while(front != side);
	}
}

void turning(Mapping *mapping, int orientation, int targetOrientation, bool walls[16][16], int address)
{	
	int frontAddress;
	if(orientation == 0)
		frontAddress = address + 4;
	else if(orientation == 1)
		frontAddress = address + 1;
	else if(orientation == 2)
		frontAddress = address - 4;
	else
		frontAddress = address- 1;

	int difference = targetOrientation - orientation;
	if(difference == -1 || difference == 3)
	{
		turn('L', 90, 50);
		usleep(20);
		if(!walls[address][frontAddress])
			checkTurn(false);

		mapping->previousAngle -= M_PI/2;
		updatePreviousEncoders(&mapping->previousLeft, &mapping->previousRight);
	}
	else if(difference == 1 || difference == -3)
	{
		turn('R', 90, 50);
		usleep(20);
		if(!walls[address][frontAddress])
			checkTurn(true);
		mapping->previousAngle += M_PI/2;
		updatePreviousEncoders(&mapping->previousLeft, &mapping->previousRight);
	}
}

void trackBack(Mapping *mapping, List *list, int address, int orientation)
{
	int difference;
	do
	{
		popNode(list);
		difference = address - list->last->address;
	}
	while(difference == 0);

	if(difference == -4)
		difference += 4;
	else if(difference == -1)
		difference += 2;
	else if(difference == 4)
		difference -= 2;
	else
		difference += 2;
	int orientationDifference = orientation - difference;
	printf("Difference = %i\torientationDifference = %i\n", difference, orientationDifference);
	printf("targetAddress = %i\n", list->last->address);
	if(orientationDifference == 2 || orientationDifference == -2)
	{
		turn('R', 180, 70);
		checkTurn(true);
		mapping->previousAngle += M_PI;
		updatePreviousEncoders(&mapping->previousLeft, &mapping->previousRight);
	}
	else if(orientationDifference == 1 || orientationDifference == -3)
	{
		turn('L', 90, 50);
		mapping->previousAngle -= M_PI/2;
		updatePreviousEncoders(&mapping->previousLeft, &mapping->previousRight);
	}
	else
	{
		turn('R', 90, 50);	
		mapping->previousAngle += M_PI/2;
		updatePreviousEncoders(&mapping->previousLeft, &mapping->previousRight);
	}
}

int decideDirection(Mapping *mapping, List *list, Point *points[16], bool walls[16][16], int address)
{
	int orientation = checkOrientation(mapping);
	printf("orientation = %i\n\n", orientation);

	if(address == 15)
	{
		popNode(list);
		turn('R', 180, 70);
		checkTurn(true);
		return 4;
	}
	else if(walls[address][address - 1] && !points[address - 1]->visited)
	{
		turning(mapping, orientation, 3, walls, address);
		return 3;
	}
	else if(walls[address][address + 4] && !points[address + 4]->visited)
	{
		turning(mapping, orientation, 0, walls, address);
		return 0;
	}
	else if(walls[address][address + 1] && !points[address + 1]->visited)
	{
		turning(mapping, orientation, 1, walls, address);
		return 1;
	}
	else if(walls[address][address - 4] && !points[address - 4]->visited)
	{
		turning(mapping, orientation, 2, walls, address);
		return 2;
	}
	trackBack(mapping, list, address, orientation);
	return 4;
}

bool visitedEverything(Point *points[16])
{
	int i;
	for(i = 0; i < 16; i++)
	{
		if(!points[i]->visited)
			return false;
	}
	return true;
}

void traverseMaze(Mapping *mapping, List *list, bool walls[16][16], Point *points[16], int speed)
{
	int address = 0;
	int orientation = 0;


	while(!visitedEverything(points))
	{
		printf("address = %i\n\n", address);
		preparePoint(mapping, points[address], 30);
		scanForWalls(mapping, list, points[address], 20, walls, address);
		printWalls(walls);
		orientation = decideDirection(mapping, list, points, walls, address);
		set_motors(0, 0);
		
		if(orientation == 0)
			address += 4;
		else if(orientation == 1)
			address += 1;
		else if(orientation == 2)
			address -= 4;
		else if(orientation == 3)
			address -= 1;
		else 
			address = list->last->address;
	}
}



int main() 
{
	connect_to_robot();
	initialize_robot();
	set_origin();

	Point *points[16];
	int i;
	for(i = 0; i < 16; i++) 
	{
		points[i] = (Point*)malloc(sizeof(Point));
	}

	bool walls[16][16];
	List *list = malloc(sizeof(List));
	Mapping *mapping = malloc(sizeof(Mapping));

	initialiseIR();
	initialiseList(list);
	initialiseWalls(walls);
	initialisePoints(points);
	initialiseMapping(mapping);

	traverseMaze(mapping, list, walls, points, 20);

	return 0;
}