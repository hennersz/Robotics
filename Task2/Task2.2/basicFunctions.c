#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include "picomms.h"
#include "straight.h"



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
        differenceLeft = abs(*leftcount - initialLeft); //represents the angle that is left
        differenceRight = abs(*rightcount - initialRight);
        percentageLeft = (target - (differenceLeft + (*leftcount - previousLeftCount)))/target;        //percentage of the angle left
        percentageRight = (target - (differenceRight + (*rightcount - previousRightCount)))/target;
        if(percentageLeft > stop || percentageRight > stop)            //if the angle is still large, turn fast
        {
            speedLeft = speed * (*leftSign);
            speedRight = speed * (*rightSign);
        }
        else if(percentageLeft < -stop || percentageRight < -stop)
        {
            speedLeft = speed * *rightSign;
            speedRight = speed * *leftSign;
        }
        else                                                          //if it approaches the end of angle, travel
        {                                                     //at propertional speed to the angle left
            speedLeft = (percentageLeft*speed*(*leftSign)) + (*leftSign);
            speedRight = (percentageRight*speed*(*rightSign)) + (*rightSign);
            if((speedLeft > -3 && speedLeft < 0) || (speedRight > 0 && speedRight < 2))
            {
                corrected = true;
                speedLeft = (*rightSign) * 3;
                speedRight = (*leftSign) * 3;
            }
            else if((speedLeft > 0 && speedLeft < 2) || (speedRight < 0 && speedRight > -2))
            {
                speedLeft = (*rightSign) * 3;
                speedRight = (*leftSign) * 3;
            }
        }
        printf("T\t%i\t%i\t%f\t%f\t%f\n", differenceLeft, differenceRight, speedLeft, speedRight, target);
        if (differenceLeft == (int)target || differenceRight == (int)target -2)                     //it reached the desired angle
        {
            if(speed > 65 && corrected == true)             //checks the angle again
            {
                corrected = false;
                break;
            }
            else if(speed < 65)
                break;
        }
        else if (differenceLeft > target && differenceRight > target)                   //travelled too far
            set_motors((int)speedLeft, (int)speedRight);
        else                                                                            //still needs to travel
            set_motors((int)speedLeft, (int)speedRight);
    }
    free(leftcount); free(rightcount); free(leftSign); free(rightSign);
}

void turn (char direction, float angle, float speed)
{
    int *left = malloc(sizeof(int));
    int *right = malloc(sizeof(int));
    get_motor_encoders(left, right);
    int initialLeft = *left;
    int initialRight = *right;
    float ratio = 2.333;                            //represents a 1 degree turn in terms of the encoder
    float encoder = ratio*angle;
    if (direction == 'L')
        turningProcess(initialLeft, initialRight, encoder,direction, speed);
    else if (direction == 'R')
        turningProcess(initialLeft, initialRight, encoder,direction, speed);
    free(left);
    free(right);
}