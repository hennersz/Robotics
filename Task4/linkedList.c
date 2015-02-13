#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "picomms.h"
#include "mapping.h"

typedef struct Node 
{
	struct Node* parent;
	int x;
	int y;
	struct Node* child;
} Node;

Node* createNode(int x, int y)
{
	Node* node = (Node*)malloc(sizeof(Node));
	node->x = x;
	node->y = y;
	return node;
}

void addNode



int main()
{
	node intitial;
	intitial.parent = NULL;
	intitial.x = intitial.y = 0;
	intitial.child = NULL;
}