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

int proportional(int *frontLeft, int *backLeft)		//calculate proportional value of how far the robot is from the wall
{
	if(*frontLeft > 50 && *backLeft > 35)          //uses both front and side sensors but filters out high values
		return 5;
	else if (*backLeft > 35)
		return *frontLeft - TARGETDISTANCE;
	else if(*frontLeft > 50)
		return *backLeft - (TARGETDISTANCE - 10);
	else
		return (*frontLeft + *backLeft)/2 - TARGETDISTANCE;
}

int differential(int *frontLeft, int *frontRight, int *backLeft, int *backRight)  //Rate of change of distance from the wall between 2 readings
{
	int previousFrontLeft = *frontLeft;
	int previousBackLeft = *backLeft;
	get_front_ir_dists(frontLeft, frontRight);
	get_side_ir_dists(backLeft, backRight);
	int currentAverage, previousAverage;
	if(*frontLeft > 50 && *backLeft > 35)	//filter out large values
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

int integral(int total, int proportionalValue)  //Integral value is sum of all the previous values
{
	total += proportionalValue;
	return total;
}

void stopped(int *lBump, int *rBump)
{												//checks to see if the robot has chrashed
	check_bumpers(lBump, rBump);
	if(*lBump == 1 || *rBump == 1)
	{
		int i = 0;
		while (i < 200)
		{
			set_motors(-10, -10);
			i++;
		}
	}
}

void checkWalls(int *frontLeft, int *frontRight, int *sideLeft, int *sideRight)	//Still not sure about this one!!
{
	if((*frontLeft < 30 && *frontRight < 30) || *frontRight < 15)  //obstacle in front
	{
		if(*sideLeft < *sideRight)
			while((*frontLeft < 30 && *frontRight < 30) || *frontRight < 15)
            {
                set_motors(10,-10);
                get_front_ir_dists(frontLeft, frontRight);
                get_side_ir_dists(sideLeft, sideRight);
            }
		else
            while((*frontLeft < 30 && *frontRight < 30) || *frontRight < 15)
            {
                set_motors(-10,10);
                get_front_ir_dists(frontLeft, frontRight);
                get_side_ir_dists(sideLeft, sideRight);
            }
	}
}

int calculateMotorValue(int *frontLeft, int *frontRight, int *backLeft, int *backRight, int integralValue, int speed)
{																					//PID algorithm
	int differentialValue = differential(frontLeft, frontRight, backLeft, backRight);
	int proportionalValue = proportional(frontLeft, backLeft);
	integralValue = integral(integralValue, proportionalValue);
	float finalValue = proportionalValue * 2.5 + differentialValue * 90 + integralValue * 0.1;
	int finalLeftSpeed = speed - finalValue;
	int finalRightSpeed = speed + finalValue;

	if(finalLeftSpeed < -127 || finalRightSpeed > 127)		//filters high or low speeds
		set_motors(-127, 127);
	else if(finalLeftSpeed > 127 || finalRightSpeed < -127)
		set_motors(127, -127);
	else if(*backRight < 5)						//if it gets too close to the other wall, turn slightly
		set_motors(speed - 2, speed + 2);
	else
		set_motors(finalLeftSpeed, finalRightSpeed);
	if (integralValue < 150 && integralValue > -150)   //Limit the impact of integral value
		return integralValue;
	else 
		return 0;
}



void wallFollower(int speed)
{
	int *frontleft = malloc(sizeof(int));
	int *frontright = malloc(sizeof(int));
	int *sideleft = malloc(sizeof(int));
	int *sideright = malloc(sizeof(int));
	int *leftBumper = malloc(sizeof(int));
	int *rightBumper = malloc(sizeof(int));
	int total = 0;
	set_ir_angle(1, -30);
	while(1)
	{
		total = calculateMotorValue(frontleft, frontright, sideleft, sideright, total, speed);
		checkWalls(frontleft, frontright, sideleft, sideright);
		stopped(leftBumper, rightBumper);
	}
	free(frontleft); free(frontright); free(sideleft); free(sideright);
	free(leftBumper); free(rightBumper);
}

int main()
{
	connect_to_robot();
	initialize_robot();

	wallFollower(40);

}