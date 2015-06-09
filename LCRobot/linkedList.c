#include <stdlib.h>
#include <stdio.h>
#include "linkedList.h"

void initialiseList(List* list)
{
	Point* firstNode = malloc(sizeof(Point));

	firstNode->x = 0;
	firstNode->y = 0;
	firstNode->address = -4;

	list->first = firstNode;
	list->last = firstNode;

	list->first->parent = NULL;
	list->last->child = NULL;
}

void initialisePoint(Point *point)
{
	point->x = 0;
	point->y = 0;
	point->parent = NULL;
	point->child = NULL;
	point->visited = false;
}

void pushNode(List *list, double x, double y, int address)
{
	Point* newNode = malloc(sizeof(Point));

	newNode->x = x;
	newNode->y = y;
	newNode->address = address;

	list->last->child = newNode;
	newNode->parent = list->last;
	list->last = newNode;
}

void pushNode2(List *list, Point *point)
{
	list->last->child = point;
	point->parent = list->last;
	list->last = point;
}

void popNode(List *list)
{
	Point *temp = list->first;
	list->first = temp->child;
	list->first->parent = NULL;
	free(temp);
}

void traverseList(List* list)
{
	Point* currentNode = list->last;
	while(currentNode!=NULL)
	{
		printf("X=%f, Y=%f, address=%d\n",currentNode->x, currentNode->y, currentNode->address);
		currentNode=currentNode->parent;
	}
}