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
void getDifference(int* left, int initialLeft, int* differenceLeft,int* right, int initialRight, int* differenceRight, int *previousLeft, int*previousRight)
{
    *previousLeft = *left;
    *previousRight = *right;
    get_motor_encoders(left, right);
    *differenceLeft = *left-initialLeft;
    *differenceRight = *right-initialRight;
}

//takes the speed and the distance you want to travel in meters
void straight(int targetSpeed, float distance)
{
    
    //intialse the pointers needed to get motor encoder values and measure how far it has turned
    int *left = malloc(sizeof(int)), *right = malloc(sizeof(int));
    int *differenceLeft = malloc(sizeof(int)), *differenceRight = malloc(sizeof(int));
    int *previousLeft = malloc(sizeof(int)), *previousRight = malloc(sizeof(int));
    int minimumSpeed = 6; // the fastest the wheels can turn without skipping encoder values
    
    int target = metersToTicks(distance);
    int stopBegin = 0.3 * target, stopEnd = 0.7 * target; float speed;
    get_motor_encoders(left, right);
    int initialLeft = *left;
    int initialRight = *right;
    
    int distanceTravelled;
    bool reachedTarget = false;
    while(!reachedTarget)
    {
        log_trail();
        getDifference(left, initialLeft, differenceLeft, right, initialRight, differenceRight, previousLeft, previousRight);
        distanceTravelled = (*differenceLeft+*differenceRight)/2;
        
        if(distanceTravelled < stopBegin)
        {
<<<<<<< HEAD
        	speed = ((float)(*differenceLeft + (*left - *previousLeft))/stopBegin) * targetSpeed + 1;
            if (speed > 127)
                speed = 127;
=======
            speedLeft = ((float)(*differenceLeft + (*left - *previousLeft))/stopBegin) * targetSpeed + 1;
            speedRight = ((float)(*differenceRight + (*right - *previousRight))/stopBegin) * targetSpeed + 1;
            if (speedLeft > 127 || speedRight > 127)
                speedRight = speedLeft = 127;
>>>>>>> FETCH_HEAD
        }
        else if(distanceTravelled > stopEnd)
        {
            speed = ((float)(target - (*differenceLeft + (*left - *previousLeft)))/target) * targetSpeed + 1;
            if (speed > 127)
                speed = 127;
            else if(speed < minimumSpeed)
                speed = minimumSpeed;
        }
        set_motors((int)speed, (int)speed);
        if(*differenceRight>=target||*differenceLeft>=target)
        {
            reachedTarget = true;
        }
    }
    free(left); free(right); free(differenceLeft); free(differenceRight); free(previousLeft); free(previousRight);
}