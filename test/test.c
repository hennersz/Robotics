#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include "dijkstra.h"
#include "picomms.h"
#include "turning.h"

#define LIMIT 50
#define WALLLIMIT 5
#define CORRECTSPEED 6
#define TURNINGSPEED 2

int MIDDLEDIST = 22;
int SENSOR_OFFSETLEFT = 1;
int SENSOR_OFFSETRIGHT = 1;
int MINIMUM_DISTANCE = 150;
int MINDIST2 = 50;

void initialisePoints(Point *points[16])
{
	int i;
	for(i = 0; i < 16; i++)
	{
		initialisePoint(points[i]);
		points[i]->x = (i % 4) * 600;
<<<<<<< HEAD
		points[i]->y = (i / 4) * 600 + 400;
=======
<<<<<<< HEAD
		points[i]->y = (i / 4) * 600 + 400;
=======
		points[i]->y = (i / 4) * 600 + 450;
>>>>>>> origin/master
>>>>>>> origin/master
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

void updateWalls(Mapping *mapping, int checkedWalls[3], bool walls[16][16], int address, int orientation)
{
	int additionArray[4] = {-1, 4, 1, -4};
	printf("WALLS LEFT: %i\t WALLS STRAIGHT: %i\t WALLS RIGHT: %i\n", checkedWalls[0], checkedWalls[1], checkedWalls[2]);

	if(address + additionArray[orientation%4] > 0 
		&& address + additionArray[orientation%4] < 16)
	{
		if(checkedWalls[0] > WALLLIMIT)
		{
			walls[address][address + additionArray[orientation%4]] = true;
			walls[address + additionArray[orientation%4]][address] = true;
		}
		else
		{
			walls[address][address + additionArray[orientation%4]] = false;
			walls[address + additionArray[orientation%4]][address] = false;
		}
	}
	if(address + additionArray[(orientation + 1)%4] > 0
		&& address + additionArray[(orientation + 1)%4] < 16)
	{
		if(checkedWalls[1] > WALLLIMIT)
		{
			walls[address][address + additionArray[(orientation + 1)%4]] = true;
			walls[address + additionArray[(orientation + 1)%4]][address] = true;
		}
		else
		{
			walls[address][address + additionArray[(orientation + 1)%4]] = false;
			walls[address + additionArray[(orientation + 1)%4]][address] = false;	
		}
	}
	if(address + additionArray[(orientation + 2)%4] > 0
		&& address + additionArray[(orientation + 2)%4] < 16)
	{
		if(checkedWalls[2] > WALLLIMIT)
		{
			walls[address][address + additionArray[(orientation + 2)%4]] = true;
			walls[address + additionArray[(orientation + 2)%4]][address] = true;
		}
		else 
		{
			walls[address][address + additionArray[(orientation + 2)%4]] = false;
			walls[address + additionArray[(orientation + 2)%4]][address] = false;	
		}
	}
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

void checkTurn(Mapping *mapping, int orientation, bool turnedRight)
{
	printf("checkTurn\n");
	double targetAngle, currentAngle;
	if(orientation == 0)
		targetAngle = 0;
	else if(orientation == 1)
		targetAngle = M_PI/2;
	else if(orientation == 2)
		targetAngle = M_PI;
	else
		targetAngle = (3.0/2.0) * M_PI;
	do
	{
		distanceTravelled(mapping);
		if(mapping->previousAngle < 0)
			currentAngle = mapping->previousAngle + 2*M_PI;
		else if(mapping->previousAngle > 2*M_PI)
			currentAngle = mapping->previousAngle - 2*M_PI;
		else
			currentAngle = mapping->previousAngle;
		if(currentAngle > targetAngle)
			set_motors(TURNINGSPEED, -TURNINGSPEED);
		else
			set_motors(-TURNINGSPEED, TURNINGSPEED);
		//printf("angle = %f\tcurrentAngle = %f\targetAngle = %f\n", mapping->previousAngle, currentAngle, targetAngle);
	}
	while(abs(currentAngle - targetAngle) > 0.2);
}

int closestWall()
{
	int left, right, i, averageLeft = 0, averageRight = 0;

	for(i = 0; i < 10; i++)
	{
		get_front_ir_dists(&left, &right);
		averageLeft += left;
		averageRight += right;
	}
	if(averageLeft < averageRight)
		return 1;
	else
		return 0;
}

void checkTurn2(Mapping *mapping, bool turnedRight)
{
	printf("checkTurn\n");
	int front, side;
	turnedRight = closestWall();

	if(turnedRight) //left sensor
	{
		do
		{
			distanceTravelled(mapping);
			front = get_front_ir_dist(0) - SENSOR_OFFSETLEFT; //front sensor is further from wall
			side = get_side_ir_dist(0);
			if(front > side)
				set_motors(-TURNINGSPEED, TURNINGSPEED);
			else
				set_motors(TURNINGSPEED, -TURNINGSPEED);

		}
		while(abs(front - side) > 1);
	}
	else   //right sensor
	{
		do
		{
			distanceTravelled(mapping);
			front = get_front_ir_dist(1) - SENSOR_OFFSETRIGHT;
			side = get_side_ir_dist(1);
			if(front > side)
				set_motors(TURNINGSPEED, -TURNINGSPEED);
			else
				set_motors(-TURNINGSPEED, TURNINGSPEED);
		}
		while(abs(front - side) > 1);
	}
}
void correctPosition(Mapping *mapping)
{
	printf("correctPosition\n");
	int distance;
	do {
		distance  = get_us_dist();
		distanceTravelled(mapping);
		if(distance > MIDDLEDIST)
			set_motors(CORRECTSPEED, CORRECTSPEED);
		else if(distance < MIDDLEDIST)
			set_motors(-CORRECTSPEED, -CORRECTSPEED);
	}
	while(distance - MIDDLEDIST != 0);
}	

void updateCoordinates(Mapping *mapping, bool left, double average, int orientation, int address)
{
	average *= 10; //convert to mm
	double difference;
	if(orientation == 0)
	{
		if(left)
			difference = (300.0 - (double)(WIDTH/2)) - average;
		else	
			difference = average - (300.0 - (double)(WIDTH/2));
		mapping->x = addressToX(address) + difference;
		mapping->y = addressToY(address);
	}
	else if(orientation == 1)
	{
		if(left)
			difference = (300.0 - (double)(WIDTH/2)) - average;
		else 
			difference = average - (300.0 - (double)(WIDTH/2));
		mapping->y = addressToY(address) + difference;	
		mapping->x = addressToX(address);
	}
	else if(orientation == 2)
	{
		if(left)
			difference = average - (300.0 - (double)(WIDTH/2));
		else	
			difference = (300.0 - (double)(WIDTH/2)) - average;
		mapping->x = addressToX(address) + difference;
		mapping->y = addressToY(address);
	}
	else if(orientation == 3)
	{
		if(left)
			difference = average - (300.0 - (double)(WIDTH/2));
		else 
			difference = (300.0 - (double)(WIDTH/2)) - average;
		printf("ADDRESS = %i\n", address);
		mapping->y = addressToY(address) + difference;	
		mapping->x = addressToX(address);
	}
}

void findOrientation(int orientation, int address, int *frontAddress, int *leftAddress, int *rightAddress)
{
	if(orientation == 0)
	{
		*frontAddress = address + 4;
		*leftAddress = address - 1;
		*rightAddress = address + 1;
	}
	else if(orientation == 1)
	{
		*frontAddress = address + 1;
		*leftAddress = address + 4;
		*rightAddress = address - 4;
	}
	else if(orientation == 2)
	{
		*frontAddress = address - 4;
		*leftAddress = address + 1;
		*rightAddress = address - 1;
	}
	else
	{
		*frontAddress = address- 1;
		*leftAddress = address - 4;
		*rightAddress = address + 4;
	}
}

void correctingCoordinates(Mapping *mapping, int address, int frontAddress, int leftAddress, int rightAddress,
 bool walls[16][16], int orientation)
{
	if((frontAddress > -1 && frontAddress < 16 && !walls[address][frontAddress])
		|| ((frontAddress < 0 || frontAddress > 15)&& frontAddress!=-4))
	{
		correctPosition(mapping);
		int front = 0, side = 0, i;
		double average;
		if(((leftAddress > -1 && leftAddress < 16 && !walls[address][leftAddress])
		|| ((leftAddress < 0 || leftAddress > 15) && leftAddress != -4)) && closestWall())
		{
			printf("USING LEFT SENSORS\n");
			for (i = 0; i<10;i++)
			{
				front += get_front_ir_dist(0);
				side += get_side_ir_dist(0);
			}
			average = (front + side)/20;
			updateCoordinates(mapping, true, average, orientation, address);
		}

		else if((rightAddress > -1 && rightAddress < 16 && !walls[address][rightAddress])
		|| ((rightAddress < 0 || rightAddress > 15) && rightAddress != -4))
		{	
			printf("USING RIGHT SENSORS\n");
			for (i = 0; i<10;i++)
			{
				front += get_front_ir_dist(1);
				side += get_side_ir_dist(1);
			}
			average = (front + side)/20;
			updateCoordinates(mapping, false, average, orientation, address);
		}
		else 
		{
			printf("UPDATED COORDINATES WITHOUT SENSORS\n");
			mapping->x = addressToX(address);
			mapping->y = addressToY(address);
		}
	}
}

void turning(Mapping *mapping, int orientation, int targetOrientation, bool walls[16][16], int address)
{	
	int frontAddress, leftAddress, rightAddress;
	findOrientation(orientation, address, &frontAddress, &leftAddress, &rightAddress);

	int difference = targetOrientation - orientation;
	if(difference == -1 || difference == 3)
	{
		turn(mapping, 'L', 90, 50);
		usleep(20);
		if(frontAddress > -1 && frontAddress < 16 && !walls[address][frontAddress])
			checkTurn(mapping, targetOrientation, false);
		else if((frontAddress < 0 || frontAddress > 15) && frontAddress != -4)
			checkTurn(mapping, targetOrientation, false);
	}
	else if(difference == 1 || difference == -3)
	{
		turn(mapping, 'R', 90, 50);
		usleep(20);
		if(frontAddress > -1 && frontAddress < 16 && !walls[address][frontAddress])
			checkTurn(mapping, targetOrientation, true);
		else if((frontAddress < 0 || frontAddress > 15) && frontAddress != -4)
			checkTurn(mapping, targetOrientation, true);
	}
	else if(difference == 2 || difference == -2)
	{
		turn(mapping, 'R', 180, 50);
		if(frontAddress > -1 && frontAddress < 16 && !walls[address][frontAddress])
			checkTurn(mapping, targetOrientation, true);
	}
}

void takingMeasurements(int checkedWalls[3])
{
	checkedWalls[0] = 0;
	checkedWalls[1] = 0;
	checkedWalls[2] = 0;
	int frontleft, frontright, sideleft, sideright;
	int usfront, i;	

	for(i = 0; i < 10; i++)
	{
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
}

void correctRobot(Mapping *mapping, bool walls[16][16], int address, int orientation)
{
	int frontAddress, leftAddress, rightAddress;
	findOrientation(orientation, address, &frontAddress, &leftAddress, &rightAddress);

	if((frontAddress > -1 && frontAddress < 16 && !walls[address][frontAddress])
		|| ((frontAddress < 0 || frontAddress > 15)&& frontAddress!=-4))
			correctPosition(mapping);

	if((leftAddress > -1 && leftAddress < 16 && !walls[address][leftAddress])
		|| ((leftAddress < 0 || leftAddress > 15) && leftAddress  != -4))
			checkTurn(mapping, orientation, true);
	else if((rightAddress > -1 && rightAddress < 16 && !walls[address][rightAddress])
		|| ((rightAddress < 0 || rightAddress > 15) && rightAddress != -4))
			checkTurn(mapping, orientation, false);

	correctingCoordinates(mapping, address, frontAddress, leftAddress, rightAddress, walls, orientation);
}

void takeMeasurements(Mapping *mapping, Point *targetPoint, bool walls[16][16], int address, int orientation)
{
	int checkedWalls[3];
	takingMeasurements(checkedWalls);
	updateWalls(mapping, checkedWalls, walls, address, orientation);
	correctRobot(mapping, walls, address, orientation);
	takingMeasurements(checkedWalls);
	updateWalls(mapping, checkedWalls, walls, address, orientation);
}

int decideDirection(Mapping *mapping, Point *points[16], bool walls[16][16], int address, int orientation)
{
	int additionArray[4] = {-1, 4, 1, -4};
	if(address == 15)
	{
		turn(mapping, 'R', 180, 70);
		checkTurn(mapping, (orientation + 2) % 4, true);
		return (orientation + 2)%4;
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
		checkTurn(mapping, (orientation + 2) % 4, true);
		return (orientation + 2) % 4;
	}
}

bool visitedEverything(Point *points[16])
{
	int i;
	for(i = 0; i < 16; i++)
	{
		if(i == 6)
		{
			i = 7;
		}
		if(!points[i]->visited)
			return false;
	}
	return true;
}

int traverseMaze(Mapping *mapping, bool walls[16][16], Point *points[16], int speed, int *orientation)
{
	int address = 0;
	
	while(true)
	{
		preparePoint(mapping, points[address], 30, *orientation);
		scanForWalls(mapping, points[address], 20, walls, address, *orientation);
		printWalls(walls);
		takeMeasurements(mapping, points[address], walls, address, *orientation);
		if(visitedEverything(points))
			break;
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

int getTargetOrientation(int orientation, int address, int targetAddress)
{
	int addressDifference = targetAddress - address;
	if(addressDifference == -1)
		return 3;
	else if(addressDifference == 1)
		return 1;
	else if(addressDifference == 4)
		return 0;
	else if(addressDifference == -4)
		return 2;
	return -1;
}

void scanForEnd(Mapping *mapping, Point *point,int speed)
{
	Point *tempPoint = malloc(sizeof(Point));
	while(!tooClose(point, mapping, MINDIST2))
	{
		goToPoint(mapping, point, tempPoint, speed, 0);
	}
	free(tempPoint);
}

void returnToStart(Mapping *mapping, List *list, bool walls[16][16], int orientation, int address)
{
	printf("returning\n");
	Point *currentNode = list->last;
	int targetOrientation, i, front = 0, side = 0;
	while(true)
	{	
		if(currentNode->address == address)
			currentNode= currentNode->parent;
		correctRobot(mapping, walls, address, orientation);
		printf("currentAddress = %i\ttargetAddress = %i\n", address, currentNode->address);
		targetOrientation = getTargetOrientation(orientation, address, currentNode->address);
		turning(mapping, orientation, targetOrientation, walls, address);
		orientation = targetOrientation;
		preparePoint(mapping, currentNode , 30, orientation);
		if(currentNode->address == -4)
			break;
		scanForEnd(mapping, currentNode, 30);
		address = currentNode->address;
		currentNode=currentNode->parent;
	}
	MIDDLEDIST = 35;
	int direction = !closestWall();
	correctPosition(mapping);
	for (i = 0; i<10;i++)
	{
		front += get_front_ir_dist(direction);
		side += get_side_ir_dist(direction);
	}
	double average = (front + side)/20;
	updateCoordinates(mapping, true, average, orientation, -4);
	mapping->y = 0;
	printf("mapping->x = %f\tmapping->y = %f\n",mapping->x, mapping->y);
}

void followList(Mapping *mapping, List *list, int speed)
{
	Point *currentNode = list->last;
	while(currentNode->parent != NULL)
	{
		preparePoint(mapping, currentNode, speed, 0);
		currentNode = currentNode->parent;
	}
	printf("Exit: currentNode->address = %i\n", currentNode->child->address);
	scanForEnd(mapping, currentNode->child, speed);
}

void crossIRSensors()
{
	set_ir_angle(0, 45);
	set_ir_angle(1, -45);
	usleep(1000000);
	set_ir_angle(0, -45);
	set_ir_angle(1, 45);
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
	initialiseSensorOffset();
	initialiseMapping(mapping);

	int address = traverseMaze(mapping, walls, points, 20, &orientation);
	
	dijkstra(walls, list, points, address, 0);
	traverseList(list);
	returnToStart(mapping, list, walls, orientation, address);
	turn(mapping, 'R', 180, 50);
	checkTurn2(mapping, true);
	set_motors(0, 0);
	crossIRSensors();
	free(list);

	list = malloc(sizeof(list));
	initialiseList(list);
	dijkstra(walls, list, points, 0, 15);
	MINIMUM_DISTANCE = 400;
	MINDIST2 = 50;
	followList(mapping, list, 50);

	return 0;
}

/*bool walls[16][16] = {{0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	
			{1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	
			{0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
			{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},
			{0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0},
			{0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0},
			{0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0}};

	*/