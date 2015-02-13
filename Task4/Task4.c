#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "picomms.h"
#include "mapping.h"
#include "linkedList.h"
#include "wallfollower.h"
#include "basicFunctions.h"

int main()
{
	List list;
	initialiseList(list);
	connect_to_robot();
	initialize_robot();
	set_origin();
	wallFollower(60, list);
	turn('L', 180, 30);
	
	return 0;

}