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

void pushNode(List *list, double x, double y)
{
	Node* newNode = malloc(sizeof(Node));

	newNode->x = x;
	newNode->y = y;

	list->last->child = newNode;
	newNode->parent = list->last;
	list->last = newNode;
}

void pushPathNode(List* list, int number)
{
	PathNode* newNode = malloc(sizeof(PathNode));

	newNode->number = number;

	list->last->child = newNode;
	newNode->parent = list->last;
	list->last = newNode;
}

void popNode(List* list, PathNode* node)
{
	if(node->parent == NULL)
	{
		list->head = node->child;
		node->child->parent = NULL;
		free(node);
	}
	else if(node->child == NULL)
	{
		list->tail = node->parent;
		node->parent->child = NULL;
		free(node);
	}
	else
	{
		node->parent->child = node->child;
		node->child->parent = node->parent;
		free(node);
	}
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