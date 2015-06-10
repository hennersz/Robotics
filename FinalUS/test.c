//Things to look at:
// - Initialise calibration! (Check comments)
// - updateCoordinates (Should we use sensor offset)
// - CorrectPosition (Shouldn't we take in consideration the USOFFSET?)

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include "picomms.h"
#include "printMaze.h"
#include "phase2.h"

#define LIMIT 50
#define WALLLIMIT 5  
#define CORRECTSPEED 6
#define TURNINGSPEED 2
#define USOFFEST 5 //9 for lily
#define HEIGHT 26

int XOFFSET = 0;		     //Used to determine XOFFSET in initialCalibration
int YOFFSET = 0;             //Used to determine YOFFSET in initialCalibration
int MIDDLEDIST = (30 - HEIGHT/2) + USOFFEST;  //Lily: 30   //This is used by US in correctPosition
int SENSOR_OFFSETLEFT = 1;
int SENSOR_OFFSETRIGHT = 1;
int MINIMUM_DISTANCE = 90;  //for preparePoint, which will stop 150 mm before target
int MINDIST2 = 30;           //for scanfForWalls, which will stop 50 mm before target

//Initialising
//-----------------------------------------------------------------
void initialisePoints(Point *points[16])
{
	//generates co-ordiates for every square in the maze, assumes robot starts
	// centrally on x axis but offcentre on y axis
	int i;
	for(i = 0; i < 16; i++)
	{
		initialisePoint(points[i]);
		points[i]->x = (i % 4) * 600 + XOFFSET;
		points[i]->y = (i / 4) * 600 + 300 + YOFFSET;
		points[i]->address = i;
	}
}

void initialiseWalls(bool walls[16][16])
{
	//sets every value to false, ie there is a wall there.
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
	//calculates where the robot has been places in the starting square
	turn(mapping, 'L', 180, 50);

	int y = get_us_dist();
	usleep(100000);
	YOFFSET = (600 - (HEIGHT/2)*10 - (y - USOFFEST)*10);         //+10; for real robot
	printf("US y measurement = %i\n", y);
	printf("Measured YOFFSET value = %i\n",YOFFSET);
	turn(mapping, 'R', 90, 50);

	int x = get_us_dist();
	usleep(100000);
	XOFFSET = (300 - (x - USOFFEST)*10 - (HEIGHT/2)*10);       //+10; for real robot
	printf("US x measurement = %i\n", x);
	printf("Measured XOFFSET value = %i\n",XOFFSET);

	turn(mapping, 'R', 90, 50);
	printf("Initial x:%f\tInitial y:%f\n", mapping->x, mapping->y);
}

void initialise(Mapping *mapping, List *list, bool walls[16][16], Point *points[16])
{
	connect_to_robot(0);
	initialize_robot();
	set_origin();

	int i;
	for(i = 0; i < 16; i++) 
	{
		points[i] = (Point*)malloc(sizeof(Point));
	}

	initialiseIR();
	initialiseList(list);
	initialiseWalls(walls);
	initialiseMapping(mapping);
	initialCalibration(mapping);
	initialisePoints(points);
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

	//gets robot angle between pi and -pi radians
	if(robotAngle > M_PI)
		robotAngle -= 2*M_PI;
	else if (robotAngle < -M_PI) 
		robotAngle +=2*M_PI;

	double angleDifference = robotAngle - targetAngle;
	
	//gets difference between pi and -pi radians
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
	//makes it so wheels only gain speed for smoother turning
	if(speedOffset > 0)
	{
		set_motors(speed, speed + (int)speedOffset);//speeds up right wheel
	}
	else
	{
		set_motors(speed - (int)speedOffset, speed);//speeds up left wheel
	}
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
//-----------------------------------------------------------------

void updateWalls(Mapping *mapping, int checkedWalls[3], bool walls[16][16], int address, int orientation)
{
	//this updates the walls around the robot, first checking the square in
	//question is not outside the grid then updating it based on measurements
	int additionArray[4] = {-1, 4, 1, -4};
	printf("WALLS LEFT: %i\t WALLS STRAIGHT: %i\t WALLS RIGHT: %i\n", checkedWalls[0], checkedWalls[1], checkedWalls[2]);

	//updates wall to the left of the robot
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

	// updates the wall in front of the robot
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

	//updates the wall to the right of the robot
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
	set_motors(0, 0);
	free(tempPoint);
}

//-----------------------------------------------------------------


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
		targetAngle = 1.5 * M_PI;
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
		{
			set_motors(TURNINGSPEED, -TURNINGSPEED);
		}
		else
		{
			set_motors(-TURNINGSPEED, TURNINGSPEED);
		}
	}
	while(fabs(currentAngle - targetAngle) > 1);
	set_motors(0,0);
}

void correctPosition(Mapping *mapping)
{
	//puts robot in middle of square but only for 1 axis!!
	printf("correctPosition\n");
	int distance;
	do {
		distance  = get_us_dist();
		distanceTravelled(mapping);
		if(distance > MIDDLEDIST)
		{
			set_motors(CORRECTSPEED, CORRECTSPEED);
		}
		else if(distance < MIDDLEDIST)
		{
			set_motors(-CORRECTSPEED, -CORRECTSPEED);
		}
	}
	while(distance - MIDDLEDIST != 0);
	set_motors(0, 0);
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

void turning(Mapping *mapping, int orientation, int targetOrientation, bool walls[16][16], int address)
{	
	int frontAddress, leftAddress, rightAddress;
	findOrientation(orientation, address, &frontAddress, &leftAddress, &rightAddress);

	int difference = targetOrientation - orientation;
	if(difference == -1 || difference == 3)
	{
		turn(mapping, 'L', 90, 50);
		usleep(20);
		checkTurn(mapping, targetOrientation, false);
	}
	else if(difference == 1 || difference == -3)
	{
		turn(mapping, 'R', 90, 50);
		usleep(20);
		checkTurn(mapping, targetOrientation, true);
	}
	else if(difference == 2 || difference == -2)
	{
		turn(mapping, 'R', 180, 50);
		checkTurn(mapping, targetOrientation, true);
	}
}

void updatingCoordinates(Mapping *mapping, int orientation, int address)
{
	if(orientation == 0 || orientation == 2)
			mapping->y = addressToY(address);
	else if(orientation == 1 || orientation == 3)
			mapping->x = addressToX(address);
}

void takingMeasurements1(int checkedWalls[3], Mapping *mapping, int orientation, int address)
{
	checkedWalls[0] = 0;
	checkedWalls[1] = 0;
	checkedWalls[2] = 0;

	int usfront, i;	

	for(i = 0; i < 10; i++)
	{
		usfront = get_us_dist();
		if(usfront > 35)
			checkedWalls[1]++;
	}
	if(checkedWalls[1] < WALLLIMIT)
	{
		correctPosition(mapping);
		updatingCoordinates(mapping, orientation, address);
	}
	turn(mapping, 'L', 90, 50);
	for(i = 0; i < 10; i++)
	{
		usfront = get_us_dist();
		if(usfront > 35)
			checkedWalls[0]++;
	}
	if(checkedWalls[0] < WALLLIMIT)
	{
		correctPosition(mapping);
		orientation = (orientation + 3)%4;
		updatingCoordinates(mapping, orientation, address);
	}
	turn(mapping, 'R', 180, 50);
	for(i = 0; i < 10; i++)
	{
		usfront = get_us_dist();
		if(usfront > 35)
			checkedWalls[2]++;
	}
	if(checkedWalls[2] < WALLLIMIT)
	{
		correctPosition(mapping);
		orientation = (orientation + 2) % 4;
		updatingCoordinates(mapping, orientation, address);
	}
	turn(mapping, 'L', 90, 50);
}

void takeMeasurements(Mapping *mapping, Point *targetPoint, bool walls[16][16], int address, int *orientation)
{
	int checkedWalls[3];
	checkTurn(mapping, *orientation, true);
	takingMeasurements1(checkedWalls, mapping, *orientation, address);
	updateWalls(mapping, checkedWalls, walls, address, *orientation);
}

int decideDirection(Mapping *mapping, Point *points[16], bool walls[16][16], int address, int orientation)
{
	int additionArray[4] = {-1, 4, 1, -4};
	if(address == 15)
	{
		turn(mapping, 'R', 180, 50);
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
		turn(mapping, 'R', 180, 50);
		checkTurn(mapping, (orientation + 2) % 4, true);
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
	
	while(true)
	{
		preparePoint(mapping, points[address], 30, *orientation);
		scanForWalls(mapping, points[address], 20, walls, address, *orientation);
		takeMeasurements(mapping, points[address], walls, address, orientation);
		printingMaze(walls);
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
	int targetOrientation;
	while(true)
	{	
		if(currentNode->address == address)
			currentNode= currentNode->parent;
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
	correctPosition(mapping);
}

void followList(Mapping *mapping, List *list, int speed)
{
	printf("Speed = %i\n", speed);
	Point *currentNode = list->last;
	while(currentNode->parent != NULL)
	{
		preparePointPhase2(mapping, currentNode, speed, 0);
		currentNode = currentNode->parent;
	}
	printf("Exit: currentNode->address = %i\n", currentNode->child->address);
	scanForEnd(mapping, currentNode->child, speed); 
	set_motors(0, 0); 
}

void crossIRSensors()
{
	set_ir_angle(0, 90);
	set_ir_angle(1, -90);
	usleep(1000000);
	set_ir_angle(0, -45);
	set_ir_angle(1, 45);
}

int main() 
{

	Point *points[16];
	bool walls[16][16];
	int orientation = 0;
	Mapping *mapping = malloc(sizeof(Mapping));
	List *list = malloc(sizeof(List));

	initialise(mapping, list, walls, points);
	
	int address = traverseMaze(mapping, walls, points, 20, &orientation);
	
	dijkstra(walls, list, points, address, 0);
	traverseList(list);
	returnToStart(mapping, list, walls, orientation, address);
	turn(mapping, 'R', 180, 50);
	set_motors(0, 0);
	crossIRSensors();
	free(list);

	list = malloc(sizeof(list));
	initialiseList(list);
	dijkstra(walls, list, points, 0, 15);
	MINIMUM_DISTANCE = 380;
	MINDIST2 = 150;
	initialCalibration(mapping);

	clock_t begin, end;
	double time_spent; 
	begin = clock();
	//NEVER GO BELOW 15 for followList!
	followList(mapping, list, 117);
	
	end = clock();
	time_spent = (double)(end - begin) / (double)CLOCKS_PER_SEC;
	printf("Time taken: %f\n", time_spent);
	return 0;
}

/*bool walls[16][16] = {{0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	
			{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	
			{0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //5
			{0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0}, //7
			{0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0}, //9
			{0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0},
			{0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0}, //12
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0}};

	*/