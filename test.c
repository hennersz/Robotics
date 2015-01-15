
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "picomms.h"

void turn (char c, float angle, int speed)
{
	int *leftcount, *rightcount;
	int test = 5;
	leftcount = rightcount = &test;
	get_motor_encoders(leftcount, rightcount);    
	int initialLeft = *leftcount;	//store initial values
	int initialRight = *rightcount;
	float ratio = 218.0/90.0;
	float encoder = ratio*angle;
	if (c == 'L')
	{
		while (1)
		{
			get_motor_encoders(leftcount, rightcount);
			printf("left = %i, right = %i\n", *leftcount - initialLeft, *rightcount - initialRight);
			if (abs(*leftcount-initialLeft) == (int)encoder && abs(*rightcount - initialRight) == (int)encoder)
				break;
			else if (abs(*leftcount-initialLeft) > encoder && abs(*rightcount - initialRight) > encoder)
				set_motors(speed, -speed);
			else
				set_motors(-speed, speed);
		}
	}
	else if (c == 'R')
		while (1)
		{
			get_motor_encoders(leftcount, rightcount);
			if (abs(*leftcount-initialLeft) == (int)encoder && abs(*rightcount - initialRight) == (int)encoder)
			{
				set_motors(speed, -speed);
				break;
			}
			else if (abs(*leftcount-initialLeft) > (int)encoder || abs(*rightcount - initialRight) > (int)encoder)
				set_motors(-speed, speed);
		}
}

void straightLine(int x, int speed)
{
	int *leftcount, *rightcount;
	int test = 5;
	leftcount = rightcount = &test;
	get_motor_encoders(leftcount, rightcount);    //find initial values
	int initialLeft = *leftcount;
	int initialRight = *rightcount;

	while(1)
	{
		get_motor_encoders(leftcount, rightcount);
		log_trail();
		if(abs(*leftcount-initialLeft) == x && abs(*rightcount - initialRight) == x)
			break;
		else if (abs(*leftcount-initialLeft) < abs(*rightcount - initialRight))
		{
			set_motors(5, 0);
			printf("correct\n");
		}
		else if (abs(*leftcount-initialLeft) > abs(*rightcount - initialRight))
		{
			set_motors(0, 5);
			printf("correct\n");
		}
		else if (abs(*leftcount-initialLeft) > x || abs(*rightcount - initialRight) > x)
			break;
		else
			set_motors(speed, speed);
	}
}
int main()
{
	connect_to_robot();
	initialize_robot();
	int i = 0;
	float angle;
	int distance;
	int speed1;
	int speed2;
	char direction;

	printf("Enter the distance\n");
	scanf("%i", &distance);
	printf("Enter speed (line)\n");
	scanf("%i", &speed1);
	printf("Enter turning speed\n");
	scanf("%i", &speed2);
	printf("Direction (L/R)\n");
	scanf(" %c", &direction);
	printf("Enter the angle\n");
	scanf("%f", &angle);
	

	while(i < 4)
	{
		straightLine(distance, speed1);
		turn(direction, angle, speed2);
		i++;
	}
}