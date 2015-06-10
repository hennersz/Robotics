#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include "picomms.h"
#include "mapping.h"
#include "linkedList.h"
#include "basicFunctions.h"


#define MAX 10000
void minDistance(bool walls[16][16], int distance[16], int point, bool visited[16])
{
	if((point + 1) < 16 && walls[point][point + 1]) //right
	{
		if(distance[point + 1] > distance[point] + 1)
		{
			distance[point + 1] = distance[point] + 1;
			//printf("Updated right: %i\n", distance[point][point] + 1);
		}
	}
	if((point - 1) > -1 && walls[point][point - 1]) //left
	{
		if(distance[point - 1] > distance[point] + 1) 
		{
			distance[point - 1] = distance[point] + 1;
			//printf("Updated left: %i\n", distance[point][point] + 1);
		}
	}
	if((point + 4) < 16 && walls[point][point + 4]) //above
	{
		if(distance[point + 4] > distance[point] + 1) 
		{
			distance[point + 4] = distance[point] + 1;
			//printf("Updated above: %i\n", distance[point][point] + 1);
		}
	}
	if((point - 4) > -1 && walls[point][point - 4]) //under
	{
		if(distance[point - 4] > distance[point] + 1) 
		{
			distance[point - 4] = distance[point] + 1;
			//printf("Updated under: %i\n", distance[point][point] + 1);
		}
	}
	visited[point] = true;
}

void initializeDistanceArray(int distance[16], bool visited[16], bool walls[16][16], int beginPoint)
{
	int i;
	for(i = 0; i < 16; i++)
	{
		distance[i]= MAX;
	}
	distance[beginPoint] = 0;

	minDistance(walls, distance, beginPoint, visited);
}

void initializeVisitedArray(bool visited[16])
{
	int i;
	for(i = 0; i < 16; i++)
		visited[i] = false;
}

int findNextClosestPoint(int distance[16], bool visited[16])
{
	int i;
	for(i = 0; i < 16; i++)
	{
		if(distance[i] != MAX && !visited[i])
			return i;
	}
	return -1;
}

int findClosestPoint(int distance[16], int point, bool walls[16][16])
{
	int minimumValue = MAX;
	int minimumAddress = point;

	if((point + 1) < 16) //right
	{
		if(distance[point + 1] < minimumValue && walls[point][point + 1])
		{
			minimumValue = distance[point + 1];
			minimumAddress = point + 1;
			//printf("Updated adress = %i\n", minimumAddress);
		}
	}
	if((point - 1) > -1) //left
	{
		if(distance[point - 1] < minimumValue  && walls[point][point - 1])
		{
			minimumValue = distance[point - 1];
			minimumAddress = point - 1;
			//printf("Updated adress = %i\n", minimumAddress);
		}
	}
	if((point + 4) < 16) //above
	{
		if(distance[point + 4] < minimumValue && walls[point][point + 4])
		{
			minimumValue = distance[point + 4];
			minimumAddress = point + 4;
			//printf("Updated adress = %i\n", minimumAddress);
		}
	}
	if((point - 4) > -1) //below
	{
		if(distance[point - 4] < minimumValue && walls[point][point - 4])
		{
			minimumValue = distance[point - 4];
			minimumAddress = point - 4;
			//printf("Updated adress = %i\n", minimumAddress);
		}
	}
	return minimumAddress;
}

void findShortestPath(List *list, int distance[16], Point *points[16], bool walls[16][16], int beginPoint, int endPoint)
{
	int address = endPoint;
	while(address != beginPoint) 
	{
		pushNode2(list, points[address]);
		address = findClosestPoint(distance, address, walls);
	}
	pushNode2(list, points[address]);
	
	Point* currentNode = list->last;
	int i = 1;
	while(currentNode!=NULL)
	{
		printf("%i: %i\n", i, currentNode->address);
		currentNode=currentNode->parent;
			i++;
	}
}

void dijkstra(bool walls[16][16], List *list, Point *points[16], int beginPoint, int endPoint)
{
	int distance[16];
	bool visited[16];
	initializeVisitedArray(visited);
	initializeDistanceArray(distance, visited, walls, beginPoint);

	int point;

	while(true)
	{
		point = findNextClosestPoint(distance, visited);
		if(point != -1)
			minDistance(walls, distance, point, visited);
		else
			break;
	}
	//printf("Exit loop\n");
	findShortestPath(list, distance, points, walls, beginPoint, endPoint);
}


