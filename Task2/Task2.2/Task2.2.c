#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "picomms.h"

#define TARGETDISTANCE 15
#define FRONTTARGET 26
#define SIDETARGET 11
#define TURNCONSTANT 35

int proportional(int *frontLeft, int *backLeft)		//calculate proportional value of how far the robot is from the wall
{
	if(*frontLeft > 50 && *backLeft > 35)          //uses both front and side sensors but filters out high values
		return 5;
	else if (*backLeft > 35)
		return *frontLeft - TARGETDISTANCE;
	else if(*frontLeft > 50)
		return *backLeft - (TARGETDISTANCE - TARGETDISTANCE/4);
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



int calculateMotorValue(int *frontLeft, int *frontRight, int *backLeft, int *backRight, int integralValue, int speed)
{																					
	int differentialValue = differential(frontLeft, frontRight, backLeft, backRight);
	int proportionalValue = proportional(frontLeft, backLeft);
	integralValue += proportionalValue;
	float finalValue = proportionalValue * 2.3 + differentialValue * 40 + integralValue * 0.06;
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
	if (integralValue < 100 && integralValue > -100)   //Limit the impact of integral value
		return integralValue;
	else 
		return 0;
}

void checkWalls(int *frontLeft, int *frontRight, int *sideLeft, int *sideRight, int speed)	//Still not sure about this one!!
{
	if((*frontLeft < FRONTTARGET && *frontRight < FRONTTARGET) || *frontRight < SIDETARGET)  //obstacle in front
	{
		int leftEncoder, rightEncoder;
		get_motor_encoders(&leftEncoder, &rightEncoder);
		if(*sideLeft < *sideRight)
			while((*frontLeft < FRONTTARGET || *frontRight < FRONTTARGET) || *frontRight < SIDETARGET)
            {
                set_motors(speed + TURNCONSTANT,speed - TURNCONSTANT);
                get_front_ir_dists(frontLeft, frontRight);
                get_side_ir_dists(sideLeft, sideRight);
            }
		else
            while((*frontLeft < FRONTTARGET || *frontRight < FRONTTARGET) || *frontRight < SIDETARGET)
            {
                set_motors(speed - TURNCONSTANT,speed + TURNCONSTANT);
                get_front_ir_dists(frontLeft, frontRight);
                get_side_ir_dists(sideLeft, sideRight);
            }
	}
}

void wallFollower(int speed)
{
	int frontleft, frontright; 
	int sideleft, sideright; 
	int leftBumper,rightBumper;
	int leftEncoder, rightEncoder;
	int total = 0;
	set_ir_angle(1, -45);
	get_motor_encoders(&leftEncoder, &rightEncoder);
	while(1)
	{
		total = calculateMotorValue(&frontleft, &frontright, &sideleft, &sideright, total, speed);
		checkWalls(&frontleft, &frontright,&sideleft, &sideright, speed);
		stopped(&leftBumper, &rightBumper);
	}
}

int main()
{
	connect_to_robot();
	initialize_robot();

	wallFollower(60);
	return 0;
}