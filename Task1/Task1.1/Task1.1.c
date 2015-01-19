#include <stdio.h>
#include "picomms.h"

void straight()
{
	int speed = 100;
    for(int i=0; i<100; i++)
    {
        set_motors(speed, speed);
    }
}

int main()
{
	connect_to_robot();
    initialize_robot();
    straight();
    return 0;
}