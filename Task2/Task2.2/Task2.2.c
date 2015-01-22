#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <math.h>
#include "picomms.h"

bool set_ir_angles(int *left, int *right, bool decreasing)
{
	if(*left == 90 && *right == 90)
	{
		*left -= 10;
		*right -= 10;
		return !decreasing;
	}
	else if(*left == -90 && *right == -90)
	{
		*left += 10;
		*right += 10;
		return !decreasing;
	}
	else if (decreasing)
	{
		*left -= 10;
		*right -= 10;
	}
	else
	{
		*left += 10;
		*right += 10;
	}
	set_ir_angle(0, *left);
	set_ir_angle(1, *right);
	return decreasing;

}
int motor(int *frontleft, int *sideleft, int* frontright, int *sideright, int speed)
{
	if(*frontleft < 10 || *sideleft < 5)
		return 127;
	else if(*frontleft < 20 || *sideleft < 10)
		return speed + 40;
	else if (*frontleft < 30 || *sideleft < 20)
		return speed + 20;
	else if(*frontright < 10 || *sideright < 5)
		return -127;
	else if(*frontright < 20 || *sideright < 10)
		return speed - 40;
	else if (*frontright < 30 || *sideright < 20)
		return speed - 20;
	else if ((*frontleft < 40 || *sideleft < 30) && (*frontright < 40 || *sideright < 30))
		return speed;
	else
		return speed - 10;
}
<<<<<<< HEAD
/*
void checkCrash(int *leftIR, int *leftEncoder, int previousLeft, int *rightIR, int *rightEncoder, int previousRight)
{
	if((*leftEncoder - previousLeft) == 0 || (*rightEncoder - previousRight == 0))
	{
		straightLine(-0.01, 10);
		turn('R', 127, 90);
	}
	while(*leftIR < 10 && *rightIR < 10)
	{
		if (*leftIR < *rightIR)
			turn('R', 127, 90);
		else
			turn('L', 127, 90);
	}
}
*/
=======

>>>>>>> parent of 9e95461... Added new function task2.2
void wallFollower(int speed)
{
	int *frontleft = malloc(sizeof(int));
	int *frontright = malloc(sizeof(int));
	int *sideleft = malloc(sizeof(int));
	int *sideright = malloc(sizeof(int));
	int *irLeft = malloc(sizeof(int));
	int *irRight = malloc(sizeof(int));
<<<<<<< HEAD
	int *leftEncoder = malloc(sizeof(int));
	int *rightEncoder = malloc(sizeof(int));

	*irRight = *irLeft = 0;
=======
	*irRight = *irLeft = -10;
>>>>>>> parent of 9e95461... Added new function task2.2
	bool decreasing = false;

	int left, right;

	while(1)
	{
		decreasing = set_ir_angles(irLeft, irRight, decreasing);
<<<<<<< HEAD
		get_motor_encoders(leftEncoder, rightEncoder);
=======
>>>>>>> parent of 9e95461... Added new function task2.2
		get_front_ir_dists(frontleft, frontright);
		get_side_ir_dists(sideleft, sideright);
		left = motor(frontleft, sideleft, frontright, sideright, speed);
		right = motor(frontright, sideright, frontleft, sideleft, speed);

		printf("LR\t%i\t%i\n", left, right);
		set_motors(left, right);
	}
}

int main()
{
	connect_to_robot();
	initialize_robot();

<<<<<<< HEAD
	wallFollower(30);
=======
	wallFollower(50);
>>>>>>> parent of 9e95461... Added new function task2.2

}