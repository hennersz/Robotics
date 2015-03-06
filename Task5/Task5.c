#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include "mapping.h"
#include "trace.h"
#include "picomms.h"
#include "wallfollower.h"


typedef struct Edge {
	bool north;
	bool south;
	bool west;
	bool east;
}Edge;

/*typedef struct Address {
	int x;
	int y;
}
*/


typedef struct Point {
	//Adress address;
	Edge *edges;
	bool visited;
	double x;
	double y;
}Point;

//phase 1: depth first search
//phase 2: return
//phase 3: dijkstra's

void initializeEdge(Edge *edge)
{
	edge->north = false;
	edge->south = false;
	edge->west = false;
	edge->east = false;
}

void initializeMaze(Point (*maze)[4][4]) 
{
	int i, j;
	for(i = 0; i < 4; i++) 
	{
		for(j = 0; j < 4; j++)
		{
			maze[i][j]->x = j * 600 + 300;
			maze[i][j]->y = i * 600 + 300;
			maze[i][j]->visited = false;
			initializeEdge(maze[i][j]->edges);
		}
	}
}

//void visitPoint()

int main() 
{
	//Point (*maze)[4][4] = NULL;
	List *list = malloc(sizeof(List));
	Mapping *mapping = malloc(sizeof(Mapping));
	
	//initializeMaze(maze);
	initialiseList(list);
	initialiseMapping(mapping);

	connect_to_robot();
	initialize_robot();
	set_origin();

	Node* node = malloc(sizeof(Node));

	node->x = 0;
	node->y = 300;

	wallFollower(50, mapping, node);
	return 0;
}