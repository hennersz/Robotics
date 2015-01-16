
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
	float ratio = 2.27;
	printf("Ratio = %f\n", ratio);
	float encoder = ratio*angle;
	if (c == 'L')
	{
		while (1)
		{
			get_motor_encoders(leftcount, rightcount);
			differenceLeft = abs(*leftcount - initialLeft);
			differenceRight = abs(*rightcount - initialRight);
			speedLeft = (((encoder - differenceLeft)/encoder)*speed) + 1;
			speedRight = (((encoder - differenceRight)/encoder)*speed) + 1;
			if(speedLeft<6&&speedRight<6)
			{
				speedLeft = 6;
				speedRight = 6;
			}

			
			

			printf("T\t%i\t%i\t%f\t%f\n", differenceLeft, differenceRight, speedLeft, speedRight);
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
			speedLeft = (((encoder - differenceLeft)/encoder)*speed) + 1;
			speedRight = (((encoder - differenceRight)/encoder)*speed) + 1;

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

	int test = 5;
	leftcount = rightcount = &test;
	get_motor_encoders(leftcount, rightcount);    //find initial values
	int initialLeft = *leftcount;
	int initialRight = *rightcount;
	float targetDistance = 1194 * distance;

	while(1)
	{
		get_motor_encoders(leftcount, rightcount);
		differenceLeft = abs(*leftcount - initialLeft);
		differenceRight = abs(*rightcount - initialRight);
		speedLeft = (((float)(targetDistance - differenceLeft)/targetDistance)*speed) + 1;
		speedRight = (((float)(targetDistance - differenceRight)/targetDistance)*speed) + 1;
		log_trail();
		printf("L\t%i\t%i\t%f\t%f\n", differenceLeft, differenceRight, speedLeft, speedRight);
		if(differenceLeft == targetDistance && differenceRight == targetDistance)
			break;
		else if (differenceLeft > targetDistance && differenceRight > targetDistance)
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
	int x;
	int delay = 10000;

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
	distance = 1.0;
	speed1 = 127;
	speed2 = 127;
	direction = 'L';
	angle = 90;
	turn('L', 720, 127);
	x = 0;
	while(x < delay)
		{
			int j = x;
			x++;
			printf("Started straightLine\n");
		}

	while(i < 4)
	{
		//straightLine(distance, speed1);
		//turn(direction, angle, speed2);
	}
}