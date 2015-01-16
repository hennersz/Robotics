#include <stdio.h>
#include "picomms.h"

void turnLeft()
{
	int speed = 100;
    for(int i = 0; i<297; i++)
    {
        set_motor(LEFT, speed);
    }
}

int main()
{
	connect_to_robot();
    initialize_robot();
    turnLeft();
    return 0;
}