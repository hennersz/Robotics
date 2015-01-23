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
#define TARGETDISTANCE 30

int proportional(int *frontLeft, int *backLeft)
{
	printf("Frontleft: %i, backLeft: %i\n", *frontLeft, *backLeft);
	if(*frontLeft > 50 && *backLeft > 35)
		return 5;
	else if (*backLeft > 35)
		return *frontLeft - TARGETDISTANCE;
	else if(*frontLeft > 50)
		return *backLeft - TARGETDISTANCE;
	else
		return (*frontLeft + *backLeft)/2 - TARGETDISTANCE;
}

int differential(int *frontLeft, int *frontRight, int *backLeft, int *backRight)
{
	int previousFrontLeft = *frontLeft;
	int previousBackLeft = *backLeft;
	get_front_ir_dists(frontLeft, frontRight);
	get_side_ir_dists(backLeft, backRight);
	int currentAverage, previousAverage;
	if(*frontLeft > 50 && *backLeft > 35)
	{
		currentAverage = 0;
		previousAverage = 0;
	}	
	else if (*backLeft > 35)
	{
		currentAverage = *frontLeft;
		previousAverage = previousFrontLeft;
	}
	else if(*frontLeft > 50)
	{
		currentAverage = *backLeft;
		previousAverage = previousBackLeft;
	}
	else
	{
		currentAverage = (*frontLeft + *backLeft)/2;
		previousAverage = (previousFrontLeft + previousBackLeft)/2;
	}
	return currentAverage - previousAverage;
}

int integral(int total, int proportionalValue)
{
	total += proportionalValue;
	return total;
}

void stopped(int *leftEncoder, int *rightEncoder)
{
	get_motor_encoders(leftEncoder, rightEncoder);
	int previousLeftEncoder = *leftEncoder;
	int previousRightEncoder = *rightEncoder;
	get_motor_encoders(leftEncoder, rightEncoder);
	if((*leftEncoder - previousLeftEncoder) < 2 || (*rightEncoder - previousRightEncoder) < 2)
	{
		int i = 0;
		while (i < 100)
		{
			set_motors(-10, -10);
			i++;
		}
	}
}

void calculateMotorValue(int *frontLeft, int *frontRight, int *backLeft, int *backRight, int total, int speed)
{
	int differentialValue = differential(frontLeft, frontRight, backLeft, backRight);
	int proportionalValue = proportional(frontLeft, backLeft);
	int integralValue = integral(total, proportionalValue);
	float finalValue = proportionalValue * 2.5 + differentialValue * 50;
	int finalLeftSpeed = speed - finalValue;
	int finalRightSpeed = speed + finalValue;
	printf("proportionlal: %i\n", proportionalValue);

	if(finalLeftSpeed < -127 || finalRightSpeed > 127)
		set_motors(-127, 127);
	else if(finalLeftSpeed > 127 || finalRightSpeed < -127)
		set_motors(127, -127);
	//else if(*frontRight < 45)
	//	set_motors(speed + 10, speed - 10);
	else
		set_motors(finalLeftSpeed, finalRightSpeed);
}


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

void wallFollower(int speed)
{
	int *frontleft = malloc(sizeof(int));
	int *frontright = malloc(sizeof(int));
	int *sideleft = malloc(sizeof(int));
	int *sideright = malloc(sizeof(int));
	int *leftEncoder = malloc(sizeof(int));
	int *rightEncoder = malloc(sizeof(int));
	int total = 0;

	while(1)
	{
		calculateMotorValue(frontleft, frontright, sideleft, sideright, total, speed);
		//stopped(leftEncoder, rightEncoder);
	}
	/*int *irLeft = malloc(sizeof(int));
	int *irRight = malloc(sizeof(int));
	

	*irRight = *irLeft = 0;
	bool decreasing = false;

	int left, right;

	while(1)
	{
		decreasing = set_ir_angles(irLeft, irRight, decreasing);
		get_motor_encoders(leftEncoder, rightEncoder);
		get_front_ir_dists(frontleft, frontright);
		get_side_ir_dists(sideleft, sideright);

		left = motor(frontleft, sideleft, frontright, sideright, speed);
		right = motor(frontright, sideright, frontleft, sideleft, speed);

		printf("LR\t%i\t%i\n", left, right);
		set_motors(left, right);
	}
	*/
}

int main()
{
	connect_to_robot();
	initialize_robot();

	wallFollower(40);

}