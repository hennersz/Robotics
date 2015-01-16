
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "picomms.h"

void turn (char c, float angle, float speed)
{
	int *leftcount, *rightcount;
	float speedLeft, speedRight;
	int differenceLeft = 0;
	int differenceRight = 0;
	int test = 5;
	leftcount = rightcount = &test;

	get_motor_encoders(leftcount, rightcount);    
	int initialLeft = *leftcount;	//store initial values
	int initialRight = *rightcount;
	float ratio = 2.33333333;
	printf("Ratio = %f\n", ratio);
	float encoder = ratio*angle;
	float percentageLeft, percentageRight;
	if (c == 'L')
	{
		while (1)
		{
			get_motor_encoders(leftcount, rightcount);
			differenceLeft = abs(*leftcount - initialLeft);
			differenceRight = abs(*rightcount - initialRight);
			percentageLeft = differenceLeft/encoder;
			percentageRight = differenceRight/encoder;
			printf("percentageLeft = %f, percentageRight = %f\n", percentageLeft, percentageRight);
			if((percentageRight>0.5)&&(percentageLeft>0.5))
			{
				speedLeft = (((encoder - differenceLeft)/encoder)*speed) + 1;
				speedRight = (((encoder - differenceRight)/encoder)*speed) + 1;
			}
			else
			{
				speedLeft = speedRight = speed;
			}

			//printf("T\t%i\t%i\t%f\t%f\n", differenceLeft, differenceRight, speedLeft, speedRight);
			if (differenceLeft == (int)encoder && differenceRight == (int)encoder)
				break;
			else if (differenceLeft > encoder && differenceRight > encoder)
				set_motors((int)speedLeft, -(int)speedRight);
			else
				set_motors(-(int)speedLeft, (int)speedRight);
		}
	}
	else if (c == 'R')
		while (1)
		{
			get_motor_encoders(leftcount, rightcount);
			differenceLeft = abs(*leftcount - initialLeft);
			differenceRight = abs(*rightcount - initialRight);
			percentageLeft = ((encoder - differenceLeft)/encoder);
			percentageRight = ((encoder - differenceRight)/encoder);
			if((percentageRight>0.7)&&(percentageLeft>0.7))
			{
				speedLeft = (((encoder - differenceLeft)/encoder)*speed) + 1;
				speedRight = (((encoder - differenceRight)/encoder)*speed) + 1;
			}
			else
			{
				speedLeft = speedRight = speed;
			}

			printf("T\t%i\t%i\t%f\t%f\n", differenceLeft, differenceRight, speedLeft, speedRight);
			if (differenceLeft == (int)encoder && differenceRight == (int)encoder)
				break;
			else if (differenceLeft > encoder && differenceRight > encoder)
				set_motors(-(int)speedLeft, (int)speedRight);
			else
				set_motors((int)speedLeft, -(int)speedRight);
		}
}

void straightLine(float distance, float speed)
{
	int *leftcount, *rightcount;
	float speedLeft, speedRight;
	int differenceLeft = 0;
	int differenceRight = 0;
	float distanceTarget = distance*1194.0;
	int test = 5;
	leftcount = rightcount = &test;
	get_motor_encoders(leftcount, rightcount);    //find initial values
	int initialLeft = *leftcount;
	int initialRight = *rightcount;
	float percentageLeft, percentageRight;
	while(1)
	{
		get_motor_encoders(leftcount, rightcount);
		differenceLeft = abs(*leftcount - initialLeft);
		differenceRight = abs(*rightcount - initialRight);
		percentageLeft = ((distanceTarget - differenceLeft)/distanceTarget);
		percentageRight = ((distanceTarget - differenceRight)/distanceTarget);
		if(percentageRight>0.9&&percentageLeft>0.9)
		{
			speedLeft = (((distanceTarget - differenceLeft)/distanceTarget)*speed) + 1;
			speedRight = (((distanceTarget - differenceRight)/distanceTarget)*speed) + 1;
		}
		else
		{
			speedLeft = speedRight = speed;
		}
		log_trail();
		printf("L\t%i\t%i\t%f\t%f\n", differenceLeft, differenceRight, speedLeft, speedRight);
		if(differenceLeft == distanceTarget && differenceRight == distanceTarget)
			break;
		else if (differenceLeft > distanceTarget && differenceRight > distanceTarget)
			set_motors(-(int)speedLeft, -(int)speedRight);
		else
			set_motors((int)speedLeft, (int)speedRight);
	}
}
int main()
{
	connect_to_robot();
	initialize_robot();
	int i = 0;
	float angle;
	float distance;
	float speed1;
	float speed2;
	char direction;

	/*printf("Enter the distance\n");
	scanf("%i", &distance);
	printf("Enter speed (line)\n");
	scanf("%f", &speed1);
	printf("Enter turning speed\n");
	scanf("%f", &speed2);
	printf("Direction (L/R)\n");
	scanf(" %c", &direction);
	printf("Enter the angle\n");
	scanf("%f", &angle);
	*/

	distance = 1;
	speed1 = 127;
	speed2 = 127;
	direction = 'L';
	angle = 90;
	turn(direction, 180, speed2);
	while(i < 4)
	{
		straightLine(distance, speed1);
		turn(direction, angle, speed2);
		i++;
	}
}