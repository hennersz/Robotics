typedef struct Node
{
	struct Node* parent;
	struct Node* child;
	float x;
	float y;
}Node;

typedef struct List
{
	Node* first;
	Node* last;
}List;

void initialiseList(List* list);
void pushNode(List *list, float x, float y);
void traverseList(List* list);