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
    set_motors(0, 0);
    free(left);
    free(right);
    free(differenceLeft);
    free(differenceRight);
    fflush(stdout);
}


float stoppingPoint (float speed)    //determines when to switch from actual speed to percentage speed
{
    if (speed < 21)
        return 0.1;
    else if (speed < 42)
        return 0.2;
    else if (speed < 75)
        return 0.3;
    else if (speed < 100)
        return 0.4;
    else
        return 0.5;
}

void wheelSigns(char direction, int* leftSign, int* rightSign)
{
    if (direction == 'L')
    {
        *leftSign = -1;
        *rightSign = 1;
    }
    else
    {
        *leftSign = 1;
        *rightSign = -1;
    }
}

void turningProcess(int initialLeft, int initialRight, float target, char direction, float speed)
{
    
    int *leftcount = malloc(sizeof(int)), *rightcount = malloc(sizeof(int)), *leftSign = malloc(sizeof(int)), *rightSign = malloc(sizeof(int));
    int previousLeftCount = 0, previousRightCount = 0,differenceRight = 0,differenceLeft = 0;
    float percentageLeft, percentageRight, speedLeft, speedRight, stop = stoppingPoint(speed);
    bool corrected = false;
    wheelSigns(direction, leftSign, rightSign);
    while (1)
    {
        previousLeftCount = *leftcount;
        previousRightCount = *rightcount;
        get_motor_encoders(leftcount, rightcount);
        differenceLeft = abs(*leftcount - initialLeft + (*leftcount - previousLeftCount));	//represents the angle that is left
        differenceRight = abs(*rightcount - initialRight + (*rightcount - previousRightCount));
        percentageLeft = ((target - differenceLeft)/target);		   //percentage of the angle left
        percentageRight = ((target - differenceRight)/target);
        if(percentageLeft > stop || percentageRight > stop)			   //if the angle is still large, turn fast
        {
            speedLeft = speed * *leftSign;
            speedRight = speed * *rightSign;
        }
        else if(percentageLeft < -stop || percentageRight < -stop)
        {
            speedLeft = speed * *rightSign;
            speedRight = speed * *leftSign;
        }
        else														  //if it approaches the end of angle, travel
        {															  //at propertional speed to the angle left
            speedLeft = (percentageLeft*speed*(*leftSign)) + *leftSign;
            speedRight = (percentageRight*speed*(*rightSign)) + *rightSign;
            if((speedLeft > -1 && speedLeft < 1) && (speedRight > -1 && speedRight < 1))
            {
                corrected = true;
                speedLeft = *rightSign;
                speedRight = *leftSign;
            }
        }
        
        if (differenceLeft == (int)target && differenceRight == (int)target) 	                //it reached the desired angle
        {
            if(speed > 65 && corrected == true)
            {
                corrected = false;
                break;
            }
            else if(speed < 65)
                break;
        }
        else if (differenceLeft > target && differenceRight > target)	        		//travelled too far
            set_motors((int)speedLeft, (int)speedRight);
        else																			//still needs to travel
            set_motors((int)speedLeft, (int)speedRight);
    }
    free(leftcount);
    free(rightcount);
    free(leftSign);
    free(rightSign);
    fflush(stdout);
}

void turn (char direction, float angle, float speed)
{
    printf("turn");
    int *left = malloc(sizeof(int));
    int *right = malloc(sizeof(int));
    get_motor_encoders(left, right);
    int initialLeft = *left;
    int initialRight = *right;
    float ratio;
    if (angle > 90)
        ratio = 2.375;							//represents a 1 degree turn in terms of the encoder
    else
        ratio = 2.366;
    float encoder = ratio*angle;
    if (direction == 'L')
        turningProcess(initialLeft, initialRight, encoder,direction, speed);
    else if (direction == 'R')
        turningProcess(initialLeft, initialRight, encoder,direction, speed);
    free(left);
    free(right);
}

int main()
{   
    connect_to_robot();
	initialize_robot();
	int i = 0;
	float angle;
	float distance;
	int speed1;
	float speed2;
	char direction;

	distance = 1.0;
	speed1 = 127;
	speed2 = 127;
	direction = 'L';
	angle = 90;
    
    turn(direction,90,speed2);
    straight(speed1, 1);
    
    //straight(speed1,1);
    turn(direction,90,speed2);
    //straight(speed1,1);
    //turn(direction,90,speed2);
    //straight(speed1,1);

    
    return 0;
}