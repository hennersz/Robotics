#include <stdbool.h>

typedef struct Point
{
	struct Point* parent;
	struct Point* child;
	double x;
	double y;
    bool visited;
    int address;
}Point;

typedef struct List
{
	Point* first;
	Point* last;
}List;

void initialiseList(List* list);
void pushNode(List *list, double x, double y, int address);
void pushNode2(List *list, Point *point);
void popNode(List *list);
void traverseList(List* list);
void initialisePoint(Point *point);