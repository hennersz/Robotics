#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include "linkedList.h"
#include "turning.h"
#include "picomms.h"
#include "test.h"

#define TURNINGSPEED 2

int decideDirectionInt(Mapping *mapping, Point *points[16], bool walls[16][16], int address, int orientation)
{
	int additionArray[4] = {-1, 4, 1, -4};
	if(address == 15)
	{
		return (orientation + 2)%4;
	}
	else if(address + additionArray[orientation%4] > -1 && //turning left
			address + additionArray[orientation%4] < 16 &&
			walls[address][address + additionArray[orientation%4]] &&  
			!points[address + additionArray[orientation%4]]->visited)
	{
		int targetOrientation = (orientation + 3) % 4;
		return targetOrientation;
	}
	else if(address + additionArray[(orientation + 1)%4] > -1 && //going straight
			address + additionArray[(orientation + 1)%4] < 16 &&
			walls[address][address + additionArray[(orientation + 1)%4]] && 
			!points[address + additionArray[(orientation + 1)%4]]->visited)
	{
		return orientation;
	}
	else if(address + additionArray[(orientation + 2)%4] > -1 && //turning right
			address + additionArray[(orientation + 2)%4] < 16 &&
			walls[address][address + additionArray[(orientation + 2)%4]] && 
			!points[address + additionArray[(orientation + 2)%4]]->visited)
	{
		int targetOrientation = (orientation + 1) % 4;
		return targetOrientation;
	}
	else if(address + additionArray[orientation%4] > -1 &&
			address + additionArray[orientation%4] < 16 &&
		walls[address][address + additionArray[orientation%4]]) {
		int targetOrientation = (orientation + 3) % 4;
		return targetOrientation;
	}
	else if(address + additionArray[(orientation + 1)%4] > -1 &&
		address + additionArray[(orientation + 1)%4] < 16 &&
		walls[address][address + additionArray[(orientation + 1)%4]]) {
		return orientation;
	}
	else if(address + additionArray[(orientation + 2)%4] > -1 &&
		address + additionArray[(orientation + 2)%4] < 16 &&
		walls[address][address + additionArray[(orientation + 2)%4]]) {
		int targetOrientation = (orientation + 1) % 4;
		return targetOrientation;
	}
	else {
		return (orientation + 2) % 4;
	}
}

void turning1(Mapping *mapping, int orientation, int targetOrientation, bool walls[16][16], int address)
{	
	int frontAddress, leftAddress, rightAddress;
	findOrientation(orientation, address, &frontAddress, &leftAddress, &rightAddress);

	int difference = targetOrientation - orientation;
	if(difference == -1 || difference == 3)
	{
		turn(mapping, 'L', 180, 50);
		usleep(20);
		checkTurn(mapping, targetOrientation, false);
	}
	else if(difference == 1 || difference == -3)
	{
		checkTurn(mapping, targetOrientation, true);
	}
	else if(difference == 2 || difference == -2)
	{
		turn(mapping, 'R', 90, 50);
		usleep(20);
		checkTurn(mapping, targetOrientation, true);
	}
	else if(difference == 0)
	{
		turn(mapping, 'L', 90, 50);
		usleep(20);
		checkTurn(mapping, targetOrientation, true);
	}
}

void turning2(Mapping *mapping, int orientation, int targetOrientation, bool walls[16][16], int address)
{	
	int frontAddress, leftAddress, rightAddress;
	findOrientation(orientation, address, &frontAddress, &leftAddress, &rightAddress);

	int difference = targetOrientation - orientation;
	if(difference == -1 || difference == 3)
	{
		checkTurn(mapping, targetOrientation, false);
	}
	else if(difference == 1 || difference == -3)
	{
		turn(mapping, 'R', 180, 50);
		usleep(20);
		checkTurn(mapping, targetOrientation, true);
	}
	else if(difference == 2 || difference == -2)
	{
		turn(mapping, 'L', 90, 50);
		usleep(20);
		checkTurn(mapping, targetOrientation, true);
	}
	else if(difference == 0)
	{
		turn(mapping, 'R', 90, 50);
		usleep(20);
		checkTurn(mapping, targetOrientation, true);
	}
}

int decideDirection1(Mapping *mapping, Point *points[16], bool walls[16][16], int address, int orientation)
{
	int additionArray[4] = {-1, 4, 1, -4};
	if(address == 15)
	{
		turn(mapping, 'R', 90, 50);
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
		turning1(mapping, orientation, targetOrientation, walls, address);
		return targetOrientation;
	}
	else if(address + additionArray[(orientation + 1)%4] > -1 && //going straight
			address + additionArray[(orientation + 1)%4] < 16 &&
			walls[address][address + additionArray[(orientation + 1)%4]] && 
			!points[address + additionArray[(orientation + 1)%4]]->visited)
	{
		printf("STRAIGHT: targetOrientation = %i\n", orientation);
		turning1(mapping, orientation, orientation, walls, address);
		return orientation;
	}
	else if(address + additionArray[(orientation + 2)%4] > -1 && //turning right
			address + additionArray[(orientation + 2)%4] < 16 &&
			walls[address][address + additionArray[(orientation + 2)%4]] && 
			!points[address + additionArray[(orientation + 2)%4]]->visited)
	{
		int targetOrientation = (orientation + 1) % 4;
		printf("RIGHT: targetOrientation = %i\n", targetOrientation);
		turning1(mapping, orientation, targetOrientation, walls, address);
		return targetOrientation;
	}
	else if(address + additionArray[orientation%4] > -1 &&
			address + additionArray[orientation%4] < 16 &&
		walls[address][address + additionArray[orientation%4]]) {
		int targetOrientation = (orientation + 3) % 4;
		printf("LEFT FOLLOWING: orientation = %i\ttargetOrientation = %i\n", orientation, targetOrientation);
		turning1(mapping, orientation, targetOrientation, walls, address);
		return targetOrientation;
	}
	else if(address + additionArray[(orientation + 1)%4] > -1 &&
		address + additionArray[(orientation + 1)%4] < 16 &&
		walls[address][address + additionArray[(orientation + 1)%4]]) {
		printf("STRAIGHT FOLLOWING: targetOrientation = %i\n", orientation);
		turning1(mapping, orientation, orientation, walls, address);
		return orientation;
	}
	else if(address + additionArray[(orientation + 2)%4] > -1 &&
		address + additionArray[(orientation + 2)%4] < 16 &&
		walls[address][address + additionArray[(orientation + 2)%4]]) {
		int targetOrientation = (orientation + 1) % 4;
		printf("RIGHT FOLLOWING: targetOrientation = %i\n", targetOrientation);
		turning1(mapping, orientation, targetOrientation, walls, address);
		return targetOrientation;
	}
	else {
		printf("TURNING: 180 degrees\n");
		turn(mapping, 'R', 90, 50);
		checkTurn(mapping, (orientation + 2) % 4, true);
		return (orientation + 2) % 4;
	}
}

int decideDirection2(Mapping *mapping, Point *points[16], bool walls[16][16], int address, int orientation)
{
	int additionArray[4] = {-1, 4, 1, -4};
	if(address == 15)
	{
		turn(mapping, 'L', 90, 50);
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
		turning2(mapping, orientation, targetOrientation, walls, address);
		return targetOrientation;
	}
	else if(address + additionArray[(orientation + 1)%4] > -1 && //going straight
			address + additionArray[(orientation + 1)%4] < 16 &&
			walls[address][address + additionArray[(orientation + 1)%4]] && 
			!points[address + additionArray[(orientation + 1)%4]]->visited)
	{
		printf("STRAIGHT: targetOrientation = %i\n", orientation);
		turning2(mapping, orientation, orientation, walls, address);
		return orientation;
	}
	else if(address + additionArray[(orientation + 2)%4] > -1 && //turning right
			address + additionArray[(orientation + 2)%4] < 16 &&
			walls[address][address + additionArray[(orientation + 2)%4]] && 
			!points[address + additionArray[(orientation + 2)%4]]->visited)
	{
		int targetOrientation = (orientation + 1) % 4;
		printf("RIGHT: targetOrientation = %i\n", targetOrientation);
		turning2(mapping, orientation, targetOrientation, walls, address);
		return targetOrientation;
	}
	else if(address + additionArray[orientation%4] > -1 &&
			address + additionArray[orientation%4] < 16 &&
		walls[address][address + additionArray[orientation%4]]) {
		int targetOrientation = (orientation + 3) % 4;
		printf("LEFT FOLLOWING: orientation = %i\ttargetOrientation = %i\n", orientation, targetOrientation);
		turning2(mapping, orientation, targetOrientation, walls, address);
		return targetOrientation;
	}
	else if(address + additionArray[(orientation + 1)%4] > -1 &&
		address + additionArray[(orientation + 1)%4] < 16 &&
		walls[address][address + additionArray[(orientation + 1)%4]]) {
		printf("STRAIGHT FOLLOWING: targetOrientation = %i\n", orientation);
		turning2(mapping, orientation, orientation, walls, address);
		return orientation;
	}
	else if(address + additionArray[(orientation + 2)%4] > -1 &&
		address + additionArray[(orientation + 2)%4] < 16 &&
		walls[address][address + additionArray[(orientation + 2)%4]]) {
		int targetOrientation = (orientation + 1) % 4;
		printf("RIGHT FOLLOWING: targetOrientation = %i\n", targetOrientation);
		turning2(mapping, orientation, targetOrientation, walls, address);
		return targetOrientation;
	}
	else {
		printf("TURNING: 180 degrees\n");
		turn(mapping, 'L', 90, 50);
		checkTurn(mapping, (orientation + 2) % 4, true);
		return (orientation + 2) % 4;
	}
}

