#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include "mapping.h"
#include "trace.h"
#include "picomms.h"
#include "wallfollower.h"

#define WIDTH 225

//phase 1: depth first search
//phase 2: return
//phase 3: dijkstra's

void initializeNodes(bool nodes[16][16])
{
	int i, j;
	for(i = 0; i < 16; i++)
	{
		for(j = 0; j < 16; j++)
		{
			nodes[i][j] = false;
		}
	}
}

void initializeVisited(bool visited[16])
{
	int i;
	for(i = 0; i < 16; i++)
	{
		visited[i] = false;
	}
}

void addressToCoor(int address, int *x, int *y) 
{
	//assuming the middle of 1 is (0, 300)
	*x = (address % 4) * 600;
	*y = (address/3) * 600 + 300;
}

int main() 
{
	List *list = malloc(sizeof(List));
	Mapping *mapping = malloc(sizeof(Mapping));
	bool nodes[16][16];
	bool visited[16]; 
	
	initializeNodes(nodes);
	initializeVisited(visited);
	initialiseList(list);
	initialiseMapping(mapping);

	connect_to_robot();
	initialize_robot();
	set_origin();

	Node* node = malloc(sizeof(Node));

	node->x = 0;
	node->y = 300 + WIDTH/2;

	wallFollower(50, mapping, node);

	return 0;
}