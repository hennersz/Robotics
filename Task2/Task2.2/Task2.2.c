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
		*left -= 20;
		*right -= 20;
		return !decreasing;
	}
	else if(*left == -90 && *right == -90)
	{
		*left += 20;
		*right += 20;
		return !decreasing;
	}
	else if (decreasing)
	{
		*left -= 20;
		*right -= 20;
	}
	else
	{
		*left += 20;
		*right += 20;
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

void wallFollower(int speed)
{
	int *frontleft = malloc(sizeof(int));
	int *frontright = malloc(sizeof(int));
	int *sideleft = malloc(sizeof(int));
	int *sideright = malloc(sizeof(int));
	int *irLeft = malloc(sizeof(int));
	int *irRight = malloc(sizeof(int));
	*irRight = *irLeft = -10;
	bool decreasing = false;

	int left, right;

	while(1)
	{
		decreasing = set_ir_angles(irLeft, irRight, decreasing);
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

	wallFollower(50);

}