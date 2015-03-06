typedef struct Node
{
	struct Node* parent;
	struct Node* child;
	double x;
	double y;
}Node;

typedef struct PathNode
{
	struct Node* parent;
	struct Node* child;
	int number;
}PathNode;

typedef struct List
{
	Node* first;
	Node* last;
}List;

void initialiseList(List* list);
void pushNode(List *list, double x, double y);
void pushPathNode(List* list, int number);
void traverseList(List* list);
void popNode(List* list, PathNode* node);