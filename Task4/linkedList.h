typedef struct Node
{
	struct Node* parent;
	struct Node* child;
	double x;
	double y;
}Node;

typedef struct List
{
	Node* first;
	Node* last;
}List;

void initialiseList(List* list);
void pushNode(List *list, double x, double y);
void traverseList(List* list);c