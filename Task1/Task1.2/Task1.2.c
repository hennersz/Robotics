#include <stdio.h>
#include "picomms.h"

void rightTurn()
{
    for (int i = 0; i<74; i++)
    {
        set_motors(100,0);
    }
}

void straight()
{
	int speed = 100;
    for(int i=0; i<100; i++)
    {
        set_motors(speed, speed);
    }
}

void square()
{
	for (int i = 0; i<4; i++)
    {
        straight();
        rightTurn();
    }
}

int main()
{
    connect_to_robot();
    initialize_robot();
    square();
    return 0;
}