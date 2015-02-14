#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <math.h>
#include "picomms.h"
#include "basicFunctions.h"

int main()
{   
    connect_to_robot();
	initialize_robot();

    //straight(50, 1);
    turn('R', 135, 127);
    straight(50, 0.707);
    turn('R', 90, 127);
    straight(50, 0.707);

    return 0;
}
