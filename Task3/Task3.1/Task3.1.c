#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "picomms.h"
#include "mapping.h"
#define TARGETDISTANCE 20
#define MAXSPEED 127
float ratio = 0.21;

/*void calculateRatio(float wheelDiam, float robotDiam)
{
	float wheelCirc = wheelDiam * M_PI;
	float robotCirc = robotDiam * M_PI;
	ratio = 1/(robotCirc/wheelCirc);
}

int findAngle(int *leftEncoder, int *rightEncoder)
{
	float temp;
	temp = (float)(*leftEncoder - *rightEncoder)/2.0 * ratio;
	return (int)temp % 360;
}

double toRadians(double angle)
{
	return (double)(angle * (M_PI/180));
}
>>>>>>> FETCH_HEAD

#define TARGETDISTANCE 35
#define STOPPINGDISTANCE 13
#define MAXSPEED 127

<<<<<<< HEAD
=======
void distanceTravelled(int *leftEncoder, int *rightEncoder, float *x, float *y)
{
	int previousLeft = *leftEncoder;
	int previousRight = *rightEncoder;
	get_motor_encoders(leftEncoder, rightEncoder);
	int angle = findAngle(leftEncoder, rightEncoder);
	float distance = (float)((*leftEncoder - previousLeft) + (*rightEncoder - previousRight)) / 2;
	//printf("Angle = %i\tDistance = %f\n", angle, distance);
	double radians = toRadians((double)angle);
	//printf("Radians = %f\n", radians);
	*y += (distance * (cos(radians))) / 12;
	*x += (distance * (sin(radians))) / 12;
	printf("x = %f\t y = %f\n", *x, *y);
} 
*/
int proportional(int *frontLeft)		//calculate proportional value of how far the robot is from the wall
{
	if(*frontLeft > 50)
		return 5;
	else
		return *frontLeft - TARGETDISTANCE;
}

int differential(int *frontLeft, int *frontRight)  //Rate of change of distance from the wall between 2 readings
{
	int previousFrontLeft = *frontLeft;
	get_front_ir_dists(frontLeft, frontRight);
	if(*frontLeft > 50)
		return 0;
	else
		return *frontLeft - previousFrontLeft;
}

void stopped(int *lBump, int *rBump)
{												//checks to see if the robot has chrashed
	check_bumpers(lBump, rBump);
	if(*lBump == 1 || *rBump == 1)
	{
		int i = 0;
		while (i < 200)
		{
			set_motors(-30, -30);
			i++;
		}
	}
}



int calculateMotorValue(int *frontLeft, int *frontRight, int *integralValue, int speed)
{																					
	int differentialValue = differential(frontLeft, frontRight);
	int proportionalValue = proportional(frontLeft);
	*integralValue +=proportionalValue;
	if (*integralValue > 100 && *integralValue < -100)   //Limit the impact of integral value
		*integralValue = 0;
	float finalValue = proportionalValue * 8 + differentialValue * 30 + *integralValue * 0.1;
	int finalSpeed = speed + finalValue;

	if(finalSpeed > MAXSPEED)		//filters high or low speeds
		finalSpeed = MAXSPEED;
	else if(finalSpeed < -MAXSPEED)
		finalSpeed = - MAXSPEED;
	return finalSpeed;
}

void checkWalls(int frontLeft, int frontRight, int speed, int finalSpeed)
{

	/*int leftEncoder, rightEncoder;
	get_motor_encoders(&leftEncoder, &rightEncoder);
	*/
	if(frontRight < 20)
		set_motors(frontRight, frontRight);
	else
		set_motors(speed, finalSpeed);
		/*while((*frontLeft < 38 && *frontRight < 38) || *frontRight < 15)
        {
            set_motors(10,-10);
            get_front_ir_dists(frontLeft, frontRight);
            //get_side_ir_dists(sideLeft, sideRight);
            //distanceTravelled(&leftEncoder, &rightEncoder, x, y);
        }
    else
        while((*frontLeft < 38 && *frontRight < 38) || *frontRight < 15)
        {
            set_motors(-10,10);
            get_front_ir_dists(frontLeft, frontRight);
            //get_side_ir_dists(sideLeft, sideRight);
            //distanceTravelled(&leftEncoder, &rightEncoder, x, y);
        }
	}
	*/
}

void wallFollower(int speed)
{
	int frontleft, frontright, finalSpeed;
	int leftBumper,rightBumper;
	int previousLeft, previousRight;
	double previousAngle = 0;
	float x= 0, y = 0;
	int total = 0;
	set_ir_angle(1, -45);
	while(1)
	{
		distanceTravelled(&previousAngle, &x, &y, &previousLeft, &previousRight);
		finalSpeed = calculateMotorValue(&frontleft, &frontright, &total, speed);
		stopped(&leftBumper, &rightBumper);
		checkWalls(frontleft, frontright, speed, finalSpeed);
		if(frontright <= STOPPINGDISTANCE)
		{
			set_motors(0, 0);
			break;
		}
	}
}

int main()
{
	connect_to_robot();
	initialize_robot();
	//calculateRatio(96.0, 260.0);
	int leftEncoder, rightEncoder;
	get_motor_encoders(&leftEncoder, &rightEncoder);
	wallFollower(50);
}