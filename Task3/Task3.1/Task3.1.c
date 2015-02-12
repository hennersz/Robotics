#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "picomms.h"
#include "mapping.h"
#include "wallfollower.h"

int main()
{
	connect_to_robot();
	initialize_robot();
	set_origin();
	wallFollower(30);
	return 0;
}