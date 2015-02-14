#include <stdlib.h>
#include <stdio.h>
#include "linkedList.h"

void initialiseList(List* list)
{
	Node* firstNode = malloc(sizeof(Node));

	firstNode->x = 0;
	firstNode->y = 0;

	list->first = firstNode;
	list->last = firstNode;

	list->first->parent = NULL;
	list->last->child = NULL;
}

void pushNode(List *list, float x, float y)
{
	Node* newNode = malloc(sizeof(Node));

	newNode->x = x;
	newNode->y = y;

	list->last->child = newNode;
	newNode->parent = list->last;
	list->last = newNode;
}

void traverseList(List* list)
{
	Node* currentNode = list->last;
	while(currentNode!=NULL)
	{
		printf("X=%f, Y=%f\n",currentNode->x, currentNode->y);
		currentNode=currentNode->parent;
	}
}