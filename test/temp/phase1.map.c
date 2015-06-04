#include "phase1.map.h"
#define RETURN 1
#define EXPLORING 0
extern double bearing;

// extern Queue* currentPath;

void printQueue(Queue* q)
{
	printf("PRINTING QUEUE\n");
	int i = 0;
	Queue* current = q;
	while (current != NULL)
	{
		int x = current->sxy.x;
		int y = current->sxy.y;
		if(x >= 0 && y >= 0) printf("Q[%d] : x %d y %d VIS %d\n", i, x, y, maze[x][y].visited);
        current = current->next;
    }
}

XY popFront(Queue** q) //pointer to pointer, because we want to modify the real queue
{
	printf("============= POP QUEUE ===========\n");
	
	printf("before\n");
	printQueue(*q);
	
	XY sxy = (*q)->sxy;
	*q = (*q)->next;
	
	printf("after\n");
	printQueue(*q);
	printf("============= ENDING ===========\n");
	
	maze[sxy.x][sxy.y].visited -= 2;
	return sxy;
}

void pushFront(Queue** q, XY sxy) //not sure about this one (i don't know if malloc will modify q or just local copy)
{
	printf("ADDING sxy x %d y %d\n", sxy.x, sxy.y);

	if(*q == NULL)
	{
		*q = malloc(sizeof(Queue));
		(*q)->sxy = sxy;
		(*q)->next = NULL;
		return;
	}
	
	Queue* new_beginning = malloc(sizeof(Queue));
	new_beginning->sxy = sxy;
	new_beginning->next = (*q);
	(*q) = new_beginning;

	maze[sxy.x][sxy.y].visited += 2;
}

int isEmpty(Queue* q)
{
	if(q == NULL) return 1;
	else return 0;
}

XY popCurrentPath(Queue* q)
{
	if(isEmpty(q))
	{
		XY cs = getCurrentSector();
		if(cs.x == 0 && cs.y == -1) return (XY){.x = -1, .y = -1}; //if we have nowhere else to go and we are already in (0,-1), that's the end
		else 
		{
			printf("\tERROR\tERROR\tERROR\tERROR\n");
			printf("%d %d \n", cs.x, cs.y);
			return (XY){.x = -100, .y = -100}; //something went wrong, that situations shoudn't occur
		}
	}
	return popFront(&q);
}

XY nextSector(Queue** currentPath)
{
	if(isEmpty(*currentPath))
	{
		printf("QUEUE IST EMPTY!!\n"); //NEIN!!!
		return (XY){.x = 0, .y = -1};
	}
	return popFront(currentPath);
}

int availSector(int x, int y, int type)
{	
	printf("AVAIL X %d Y %d type %d\n", x, y, type);
	if(x > MAZE_WIDTH - 1 || x < 0 || y > (MAZE_HEIGHT - 1) || y < 0) return 0;
	if(maze[x][y].visited != 0 && type == EXPLORING) return 0; //checking if visited only if i want to explore it - if that's my return path, do not do that
	return 1;
}

double convertToDegrees(double radians)
{
	if(radians > 0) return radians*180/M_PI;
	else return 360 + radians*180/M_PI;
}

void updateSector(Queue** currentPath)
{
	printQueue(*currentPath);
	
	if((*currentPath) == NULL) printf("XX\n");
	
	XY currSect = getCurrentSector();
	
	int x = currSect.x;
	int y = currSect.y;
	
	if(maze[x][y].visited != 1)
	{
		maze[x][y].visited = 1;
		set_ir_angle(LEFT, -45);
		set_ir_angle(RIGHT, 45);  
		usleep(500000);
		double frontLeftReading, frontRightReading, sideLeftReading, sideRightReading; 
		int ultraSound;
		getFrontIR(&frontLeftReading, &frontRightReading);
		getSideIR(&sideLeftReading, &sideRightReading);
		ultraSound = get_us_dist();
		//printf("frontleft: %d, frontRight: %d, sideLeft: %d, sideRight: %d, US: %d", frontLeftReading, frontRightReading, sideLeftReading,sideRightReading,ultraSound);
		int northWall = 0;
		int southWall = 0;
		int westWall = 0;
		int eastWall = 0;
		
		int northType = 0;
		int southType = 0;
		int westType = 0;
		int eastType = 0;
		
// 		a[i] = a[(i + bearing*2/M_PI)%4]
		
		if(convertToDegrees(bearing) < 15 || convertToDegrees(bearing) > 345) //first case - it's heading north
		{
				
			if((frontLeftReading < SECTOR_WIDTH/2) || (sideLeftReading < SECTOR_WIDTH/2))
			{
				westWall = 1;
			}
			if((frontRightReading < SECTOR_WIDTH/2) || (sideRightReading < SECTOR_WIDTH/2))
			{
				eastWall = 1;
			}
			if(ultraSound < SECTOR_WIDTH/2)
			{
				northWall = 1;
			}
			southType = RETURN;
			pushFront(currentPath, (XY){.x = x, .y = y-1}); 
		}
		
		if(convertToDegrees(bearing) < 105 && convertToDegrees(bearing) > 75) //first case - it's heading north
		{
				
			if((frontLeftReading < SECTOR_WIDTH/2) || (sideLeftReading < SECTOR_WIDTH/2))
			{
				northWall = 1;
			}
			if((frontRightReading < SECTOR_WIDTH/2) || (sideRightReading < SECTOR_WIDTH/2))
			{
				southWall = 1;
			}
			if(ultraSound < SECTOR_WIDTH/2)
			{
				eastWall = 1;
			}
			westType = RETURN;
			pushFront(currentPath, (XY){.x = x-1, .y = y});
		}
		
		if(convertToDegrees(bearing) < 195 && convertToDegrees(bearing) > 165) //first case - it's heading north
		{
				
			if((frontLeftReading < SECTOR_WIDTH/2) || (sideLeftReading < SECTOR_WIDTH/2))
			{
				eastWall = 1;
			}
			if((frontRightReading < SECTOR_WIDTH/2) || (sideRightReading < SECTOR_WIDTH/2))
			{
				westWall = 1;
			}
			if(ultraSound < SECTOR_WIDTH/2)
			{
				southWall = 1;
			}
			northType= RETURN;
			pushFront(currentPath, (XY){.x = x, .y = y+1});
		}
		
		if(convertToDegrees(bearing) < 285 && convertToDegrees(bearing) > 255) //first case - it's heading north
		{
				
			if((frontLeftReading < SECTOR_WIDTH/2) || (sideLeftReading < SECTOR_WIDTH/2))
			{
				southWall = 1;
			}
			if((frontRightReading < SECTOR_WIDTH/2) || (sideRightReading < SECTOR_WIDTH/2))
			{
				northWall = 1;
			}
			if(ultraSound < SECTOR_WIDTH/2)
			{
				westWall = 1;
			}
			eastType= RETURN;
			pushFront(currentPath, (XY){.x = x+1, .y = y});
		}
		
		printf("bearing: %.2f toDegree: %.2f\n", bearing, convertToDegrees(bearing));
		
		printf("DEBUG TYP\tN %d S %d E %d W %d\n", northType, southType, eastType, westType);
		printf("DEBUG WAL\tN %d S %d E %d W %d\n", northWall, southWall, eastWall, westWall);
		printf("DEBUG ULT\t%d\n", ultraSound);
// 		/*rotate(&northWall, &westWall, &southWall, &eastWall, bearing);*/
		
// 		set_ir_angle(LEFT, 45);
// 		set_ir_angle(RIGHT, -45);  
// 		sleep(1);
// 		get_front_ir_dists(&frontLeftReading, &frontRightReading);
		
		//////////////////////////////////////////////////
		
		maze[x][y] = (Sector){.northWall = northWall, .southWall = southWall, .eastWall = eastWall, .westWall = westWall, .visited = 1};
		
		if(northWall == 0 && availSector(x, y+1, EXPLORING) == 1) pushFront(currentPath, (XY){.x = x, .y = y+1});
		if(southWall == 0 && availSector(x, y-1, EXPLORING) == 1) pushFront(currentPath, (XY){.x = x, .y = y-1});
		if(eastWall == 0 && availSector(x+1, y, EXPLORING) == 1) pushFront(currentPath, (XY){.x = x+1, .y = y});
		if(westWall == 0 && availSector(x-1, y, EXPLORING) == 1) pushFront(currentPath, (XY){.x = x-1, .y = y});
		
	}
}