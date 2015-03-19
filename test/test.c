#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include "dijkstra.h"
#include "picomms.h"
//#include "basicFunctions.h"
#include "turning.h"

#define LIMIT 50
#define WIDTH 225
#define WALLLIMIT 10
#define WHEELDIAM 96
#define TURNINGSPEED 1
#define SENSOR_OFFSET 1

int MINIMUM_DISTANCE = 150;
int MINDIST2 = 30;

void initialisePoints(Point *points[16])
{
	int i;
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
	double deltaX = point->x - mapping->x;
	double deltaY = point->y - mapping->y;
	double targetAngle = atan2(deltaX, deltaY);
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

int calculateSpeedOffset(Mapping *mapping, Point *point, int orientation)
{
	double targetAngle = findTargetAngle(mapping, point);
	double robotAngle = mapping->previousAngle;
	if(robotAngle > M_PI)
	{
		robotAngle -= 2*M_PI;
	}
	else if (robotAngle < -M_PI) 
	{
		robotAngle +=2*M_PI;
	}
	double angleDifference = robotAngle - targetAngle;
	
	if(angleDifference > M_PI)
		angleDifference -= 2*M_PI;
	else if(angleDifference < -M_PI)
		angleDifference += 2*M_PI;

	int speedOffset = (int)toDegrees(angleDifference);
	if(speedOffset > LIMIT)
		speedOffset = LIMIT;
	else if(speedOffset < -LIMIT)
		speedOffset = -LIMIT;

	return speedOffset;
}

void goTo(Mapping *mapping, Point *point, int speed, int orientation)
{
	distanceTravelled(mapping);
	double speedOffset = (double)calculateSpeedOffset(mapping, point, orientation);
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
	point->x = mapping->x + (MINIMUM_DISTANCE * cos(angle));
	point->y = mapping->y + (MINIMUM_DISTANCE * sin(angle));

	//set_point(point->x/10, point->y/10);
}

void goToPoint(Mapping *mapping, Point *targetPoint, Point *tempPoint, int speed, int orientation)
{
	findNextPoint(mapping, tempPoint, targetPoint); 
	goTo(mapping, tempPoint, speed, orientation);
}

void preparePoint(Mapping *mapping, Point *targetPoint, int speed, int orientation)
{
	Point *tempPoint = malloc(sizeof(Point));
	initialisePoint(tempPoint);
	set_point((int)targetPoint->x/10, (int)targetPoint->y/10);
	printf("targetPoint: x = %f, y = %f\n", targetPoint->x, targetPoint->y);

	while(!tooClose(targetPoint, mapping, MINIMUM_DISTANCE))
	{
		goToPoint(mapping, targetPoint, tempPoint, speed, orientation);
	}	
	free(tempPoint);
}

void updateWalls(Mapping *mapping, int checkedWalls[3], bool walls[16][16], int address, int orientation)
{
	int additionArray[4] = {-1, 4, 1, -4};
	printf("WALLS LEFT: %i\t WALLS STRAIGHT: %i\t WALLS RIGHT: %i\n", checkedWalls[0], checkedWalls[1], checkedWalls[2]);

	if(checkedWalls[0] > WALLLIMIT && address + additionArray[orientation%4] > 0 
		&& address + additionArray[orientation%4] < 16)
	{
		walls[address][address + additionArray[orientation%4]] = true;
		walls[address + additionArray[orientation%4]][address] = true;
	}
	if(checkedWalls[1] > WALLLIMIT && address + additionArray[(orientation + 1)%4] > 0
		&& address + additionArray[(orientation + 1)%4] < 16)
	{
		walls[address][address + additionArray[(orientation + 1)%4]] = true;
		walls[address + additionArray[(orientation + 1)%4]][address] = true;
	}
	if(checkedWalls[2] > WALLLIMIT && address + additionArray[(orientation + 2)%4] > 0
		&& address + additionArray[(orientation + 2)%4] < 16)
	{
		walls[address][address + additionArray[(orientation + 2)%4]] = true;
		walls[address + additionArray[(orientation + 2)%4]][address] = true;
	}
}


void scanForWalls(Mapping *mapping, Point *targetPoint, int speed, bool walls[16][16], int address, int orientation)
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
		goToPoint(mapping, targetPoint, tempPoint, speed, orientation);
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
	updateWalls(mapping, checkedWalls, walls, address, orientation);
	free(tempPoint);
	targetPoint->visited = true;
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
	if(address == -1)
		return;
	else if(orientation == 0)
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
		turn(mapping, 'L', 90, 50);
		usleep(20);
		if(frontAddress > -1 && frontAddress < 16 && !walls[address][frontAddress])
			checkTurn(false);
	}
	else if(difference == 1 || difference == -3)
	{
		turn(mapping, 'R', 90, 50);
		usleep(20);
		if(frontAddress > -1 && frontAddress < 16 && !walls[address][frontAddress])
			checkTurn(true);
	}
}

int decideDirection(Mapping *mapping, Point *points[16], bool walls[16][16], int address, int orientation)
{
	int additionArray[4] = {-1, 4, 1, -4};
	if(address == 15)
	{
		turn(mapping, 'R', 180, 70);
		checkTurn(true);
		return 2;
	}
	else if(address + additionArray[orientation%4] > -1 && //turning left
			address + additionArray[orientation%4] < 16 &&
			walls[address][address + additionArray[orientation%4]] &&  
			!points[address + additionArray[orientation%4]]->visited)
	{
		int targetOrientation = (orientation + 3) % 4;
		printf("LEFT: orientation = %i\ttargetOrientation = %i\n", orientation, targetOrientation);
		turning(mapping, orientation, targetOrientation, walls, address);
		return targetOrientation;
	}
	else if(address + additionArray[(orientation + 1)%4] > -1 && //going straight
			address + additionArray[(orientation + 1)%4] < 16 &&
			walls[address][address + additionArray[(orientation + 1)%4]] && 
			!points[address + additionArray[(orientation + 1)%4]]->visited)
	{
		printf("STRAIGHT: targetOrientation = %i\n", orientation);
		turning(mapping, orientation, orientation, walls, address);
		return orientation;
	}
	else if(address + additionArray[(orientation + 2)%4] > -1 && //turning right
			address + additionArray[(orientation + 2)%4] < 16 &&
			walls[address][address + additionArray[(orientation + 2)%4]] && 
			!points[address + additionArray[(orientation + 2)%4]]->visited)
	{
		int targetOrientation = (orientation + 1) % 4;
		printf("RIGHT: targetOrientation = %i\n", targetOrientation);
		turning(mapping, orientation, targetOrientation, walls, address);
		return targetOrientation;
	}
	else if(address + additionArray[orientation%4] > -1 &&
			address + additionArray[orientation%4] < 16 &&
		walls[address][address + additionArray[orientation%4]]) {
		int targetOrientation = (orientation + 3) % 4;
		printf("LEFT FOLLOWING: orientation = %i\ttargetOrientation = %i\n", orientation, targetOrientation);
		turning(mapping, orientation, targetOrientation, walls, address);
		return targetOrientation;
	}
	else if(address + additionArray[(orientation + 1)%4] > -1 &&
		address + additionArray[(orientation + 1)%4] < 16 &&
		walls[address][address + additionArray[(orientation + 1)%4]]) {
		printf("STRAIGHT FOLLOWING: targetOrientation = %i\n", orientation);
		turning(mapping, orientation, orientation, walls, address);
		return orientation;
	}
	else if(address + additionArray[(orientation + 2)%4] > -1 &&
		address + additionArray[(orientation + 2)%4] < 16 &&
		walls[address][address + additionArray[(orientation + 2)%4]]) {
		int targetOrientation = (orientation + 1) % 4;
		printf("RIGHT FOLLOWING: targetOrientation = %i\n", targetOrientation);
		turning(mapping, orientation, targetOrientation, walls, address);
		return targetOrientation;
	}
	else {
		printf("TURNING: 180 degrees\n");
		turn(mapping, 'R', 180, 70);
		checkTurn(true);
		return (orientation + 2) % 4;
	}
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

int traverseMaze(Mapping *mapping, bool walls[16][16], Point *points[16], int speed, int *orientation)
{
	int address = 0;
	
	while(!visitedEverything(points))
	{
		preparePoint(mapping, points[address], 30, *orientation);
		scanForWalls(mapping, points[address], 20, walls, address, *orientation);
		printWalls(walls);
		printf("BEFORE currentOrientation = %i\taddress = %i\n\n", *orientation, address);
		*orientation = decideDirection(mapping, points, walls, address, *orientation);
		set_motors(0, 0);
		
		if(*orientation == 0)
			address += 4;
		else if(*orientation == 1)
			address += 1;
		else if(*orientation == 2)
			address -= 4;
		else if(*orientation == 3)
			address -= 1;

		printf("AFTER currentOrientation = %i\taddress = %i\n\n", *orientation, address);
	}
	return address;
}

////////////////////////////////henrys return code///////////////////////////////////////////


int getOrientation(double angle)
{
	//angles are between Pi and -Pi
	if(angle>(-M_PI)*0.25&&angle<=M_PI*0.25)
	{
		return 0;
	}
	else if(angle>M_PI*0.25&&angle<=M_PI*0.75)
	{
		return 1;
	}
	else if ((angle>M_PI*0.75&&angle<=M_PI)||(angle<(-M_PI)*0.75&&angle>-M_PI))
	{
		return 2;
	}
	else
	{
		return 3;
	}
}

int getTargetOrientation(Mapping *mapping, Point *targetPoint)
{
	double dx = targetPoint->x - mapping->x;
	double dy = targetPoint->y - mapping->y;

	double targetAngle = atan2(dx,dy);
	int targetOrientation = getOrientation(targetAngle);

	return targetOrientation;
}

int getAddress(Mapping *mapping)
{
	int x = (int)(mapping->x/600);
	int y = (int)(mapping->y/600);

	int address = x + 4*y;

	return address;
}



void returnToStart(Mapping *mapping, List *list, bool walls[16][16])
{
	printf("returning\n");
	Point *currentNode = list->last;
	int address, orientation, targetOrientation;
	while(currentNode != NULL)
	{
		address = getAddress(mapping);
		orientation = getOrientation(mapping->previousAngle);
		targetOrientation = getTargetOrientation(mapping, currentNode);
		turning(mapping, orientation, targetOrientation, walls, address);
		orientation = getOrientation(mapping->previousAngle);
		preparePoint(mapping, currentNode , 30, orientation);
		currentNode=currentNode->parent;
	}
	/*
	Point *tempPoint = malloc(sizeof(Point));
	initialisePoint(tempPoint);

	while(!tooClose(currentNode, mapping, MINDIST2))
	{
		goToPoint(mapping, currentNode, tempPoint, 30, orientation);
	}	
	free(tempPoint);
	/
	*/
}

void scanForEnd(Mapping *mapping, Point *point,int speed)
{
	Point *tempPoint = malloc(sizeof(Point));
	while(!tooClose(point, mapping, MINDIST2))
	{
		goToPoint(mapping, point, tempPoint, speed, 0);
	}
}

void followList(Mapping *mapping, List *list, int speed)
{
	Point *currentNode = list->last;
	while(currentNode != NULL)
	{
		preparePoint(mapping, currentNode, speed, 0);
		currentNode = currentNode->parent;
	}
	scanForEnd(mapping, list->first, speed);
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
	int orientation = 0;
	Mapping *mapping = malloc(sizeof(Mapping));
	List *list = malloc(sizeof(List));

	initialiseIR();
	initialiseList(list);
	initialiseWalls(walls);
	initialisePoints(points);
	initialiseMapping(mapping);

	int address = traverseMaze(mapping, walls, points, 20, &orientation);
	
	/*
	bool walls[16][16] = {{0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	
			{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	
			{0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
			{0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0},
			{0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0},
			{0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0}};
	*/
	dijkstra(walls, list, points, address, 0);
	traverseList(list);
	returnToStart(mapping, list, walls);
	turn(mapping, 'R', 180, 50);
	checkTurn(true);
	//return to 0
	// then turn till orientation is 2
	//go to (0, 0)
	//turn 180 degrees
	//checkturn
	//cross IR
	//empty list
	free(list);
	list = malloc(sizeof(list));
	initialiseList(list);
	dijkstra(walls, list, points, 0, 15);
	traverseList(list);
	popNode(list);
	traverseList(list);
	printf("address:%i\n", list->first->address);
	MINIMUM_DISTANCE = 300;
	MINDIST2 = 50;
	followList(mapping, list, 50);
	
	//go to 15

	return 0;
}