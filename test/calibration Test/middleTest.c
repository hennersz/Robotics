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
#define WALLLIMIT 5 
#define CORRECTSPEED 6
#define TURNINGSPEED 2
#define USOFFEST 8	//cm or mm?

#define HEIGHT 260  //If height == width 

int MIDDLEDIST = 22;  //Lily: 30
int SENSOR_OFFSETLEFT = 1;
int SENSOR_OFFSETRIGHT = 1;
int MINIMUM_DISTANCE = 380;
int MINDIST2 = 50;

//Initialising
//-----------------------------------------------------------------
void initialisePoints(Point *points[16])
{
	int i;
	for(i = 0; i < 16; i++)
	{
		initialisePoint(points[i]);
		points[i]->x = (i % 4) * 600;
		points[i]->y = (i / 4) * 600 + 400; //300?
		points[i]->address = i;
	}
}

void initialiseSensorOffset()
{
	int sideLeft, sideRight;
	int frontLeft, frontRight;
	int leftTotal = 0, rightTotal = 0;
	int i;
	for(i = 0; i < 10; i++)
	{
		get_side_ir_dists(&sideLeft, &sideRight);
		get_front_ir_dists(&frontLeft, &frontRight);
		leftTotal += (frontLeft - sideLeft);
		rightTotal += (frontRight - sideRight);
	}

	SENSOR_OFFSETLEFT = round((double)leftTotal / 20);
	SENSOR_OFFSETRIGHT = round((double)rightTotal / 20);
	if(SENSOR_OFFSETRIGHT > 5)
		SENSOR_OFFSETRIGHT = 5;
	if(SENSOR_OFFSETLEFT > 5)
		SENSOR_OFFSETLEFT = 5;
	printf("SENSOR_OFFSETLEFT = %i\tSENSOR_OFFSETRIGHT = %i\n", SENSOR_OFFSETLEFT, SENSOR_OFFSETRIGHT);
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

void initialCalibration(Mapping *mapping)
{
	turn(mapping, 'L', 180, 50);
	int y = get_us_dist();
	if (y>40)
	usleep(100000);
	printf("Measured y value = %i\n",(y+USOFFEST)*10);
	mapping->y = (y + USOFFEST)*10-450;
	//mapping->y = (y+USOFFEST)*10 + HEIGHT/2 - 600;
	turn(mapping, 'R', 90, 50);
	int x = get_us_dist();
	usleep(100000);
	printf("Measured x value = %i\n",(x+USOFFEST)*10);
	//mapping->x = (x+USOFFEST)*10 + HEIGHT/2 - 300;
	mapping->x = (x + USOFFEST)*10-300;
	turn(mapping, 'R', 90, 50);
	printf("Initial x:%f\tInitial y:%f\n", mapping->x, mapping->y);
}

void initialise(Mapping *mapping, List *list, bool walls[16][16], Point *points[16])
{
	initialiseIR();
	initialiseList(list);
	initialiseWalls(walls);
	initialisePoints(points);
	//initialiseSensorOffset();              not reliable for real robot
	initialiseMapping(mapping);
	initialCalibration(mapping);
}

//end of initialising
//-----------------------------------------------------------------
//Travelling to points

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

	set_point(mapping->x/10, mapping->y/10);
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

void scanForWalls(Mapping *mapping, Point *targetPoint, int speed, bool walls[16][16], int address, int orientation)
{
	Point *tempPoint = malloc(sizeof(Point));
	initialisePoint(tempPoint);
	while(!tooClose(targetPoint, mapping, MINDIST2))
	{
		goToPoint(mapping, targetPoint, tempPoint, speed, orientation);
	}
	targetPoint->visited = true;
	free(tempPoint);
}

int traverseMaze(Mapping *mapping, bool walls[16][16], Point *points[16], int speed, int *orientation)
{
	int address = 0;
	int temp = 1;
	preparePoint(mapping, points[address], 30, *orientation);
	printf("Done preparePoint, enter a number to continue\n");
	set_motors(0, 0);
	scanf("%i", &temp);
	scanForWalls(mapping, points[address], 20, walls, address, *orientation);
	address = 4;
	printf("Done scanForWalls, enter a number to continue\n");
	set_motors(0, 0);
	scanf("%i", &temp);
	preparePoint(mapping, points[address], 30, *orientation);
	printf("Done preparePoint, enter a number to continue\n");
	set_motors(0, 0);
	scanf("%i", &temp);
	scanForWalls(mapping, points[address], 20, walls, address, *orientation); 

	return address;
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

	initialise(mapping, list, walls, points);
	
	traverseMaze(mapping, walls, points, 20, &orientation);
	return 0;
}
