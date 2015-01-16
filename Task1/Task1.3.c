
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
	float ratio = 214/90.0;
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

void straightLine(int x, float speed)
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

	while(1)
	{
		get_motor_encoders(leftcount, rightcount);
		differenceLeft = abs(*leftcount - initialLeft);
		differenceRight = abs(*rightcount - initialRight);
		speedLeft = (((float)(x - differenceLeft)/x)*speed) + 1;
		speedRight = (((float)(x - differenceRight)/x)*speed) + 1;
		log_trail();
		printf("T\t%i\t%i\t%f\t%f\n", differenceLeft, differenceRight, speedLeft, speedRight);
		if(differenceLeft == x && differenceRight == x)
			break;
		else if (differenceLeft > x && differenceRight > x)
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
	int distance;
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
	distance = 1000;
	speed1 = 127;
	speed2 = 127;
	direction = 'L';
	angle = 360;
	//while(i < 6)
	//{
		straightLine(distance, speed1);
		turn(direction, angle, speed2);
		straightLine(distance, -speed1);
	//	i++;
	//}
}