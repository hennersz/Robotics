#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include "picomms.h"

int calculateAcceleration(int startSpeed, int endSpeed, int distance)
{
	int difference = endSpeed - startSpeed;
	int acceleration = difference/distance;
	
	return acceleration;
}

int metersToTicks(float distance)//takes distance in meters
{
    float ticks = 1194*distance; //1194 is the ratio from ticks to meters
    return (int)ticks;
}
void getDifference(int* left, int initialLeft, int* differenceLeft,int* right, int initialRight, int* differenceRight)
{
    get_motor_encoders(left, right);
    *differenceLeft = *left-initialLeft;
    *differenceRight = *right-initialRight;
}

//takes the speed and the distance you want to travel in meters
void straight(int targetSpeed, float distance)
{
    
	//intialse the pointers needed to get motor encoder values and measure how far it has turned
    int *left = malloc(sizeof(int));
    int *right = malloc(sizeof(int));
    int *differenceLeft = malloc(sizeof(int));
    int *differenceRight = malloc(sizeof(int));

    int minimumSpeed = 6; // the fastest the wheels can turn without skipping encoder values

    int target = metersToTicks(distance);
    int accelerationZone = 0.1*target;
    int deccelerationZone = 0.9*target;
    int acceleration = calculateAcceleration(0,targetSpeed,accelerationZone);
    int decelleration = calculateAcceleration(targetSpeed,minimumSpeed,accelerationZone);

    int speed = 0;

    get_motor_encoders(left, right);
    int initialLeft = *left;
    int initialRight = *right;

    int distanceTravelled;
    bool reachedTarget = false;
    printf("straight");

    while(!reachedTarget)
    {
        log_trail();
        getDifference(left, initialLeft, differenceLeft, right, initialRight, differenceRight);
        distanceTravelled = (*differenceLeft+*differenceRight)/2;
        if(speed<targetSpeed)
        {
        	log_trail();
        	speed+=acceleration;
        }
        else if(distanceTravelled>=deccelerationZone && speed>minimumSpeed)
        {
        	log_trail();
        	speed+=decelleration;
        }
        set_motors(speed, speed);
        if(*differenceRight>=target||*differenceLeft>=target)
        {
            reachedTarget = true;
        }
    }
    set_motors(0,0);
    free(left);
    free(right);
    free(differenceLeft);
    free(differenceRight);
}