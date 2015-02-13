#include <stdlib.h>
#include <stdio.h>

typedef struct Node
{
	struct Node* parent;
	struct Node* child;
	int x;
	int y;
}Node;

typedef struct List
{
	Node* first;
	Node* last;
}List;

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

void pushNode(List *list, int x, int y)
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
		printf("X=%i, Y=%i\n",currentNode->x, currentNode->y);
		currentNode=currentNode->parent;
	}
}