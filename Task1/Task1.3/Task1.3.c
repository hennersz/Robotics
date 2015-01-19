#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include "picomms.h"

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
void turningProcess(int initialLeft, int initialRight, float target, int leftSign, int rightSign, float speed, bool turning)
{
    
    int *leftcount = malloc(sizeof(int));
    int *rightcount = malloc(sizeof(int));
    int previousLeftCount, previousRightCount;
    previousRightCount = previousLeftCount = 0;
    float percentageLeft, percentageRight;
    float speedLeft, speedRight;
    int differenceLeft = 0;
    int differenceRight = 0;
    float stop = stoppingPoint(speed);
    bool corrected = false;
    if (!turning)
        corrected = true;
    while (1)
    {
        log_trail();
        previousLeftCount = *leftcount;
        previousRightCount = *rightcount;
        get_motor_encoders(leftcount, rightcount);
        differenceLeft = abs(*leftcount - initialLeft + (*leftcount - previousLeftCount));	//represents the angle that is left
        differenceRight = abs(*rightcount - initialRight + (*rightcount - previousRightCount));
        percentageLeft = ((target - differenceLeft)/target);		   //percentage of the angle left
        percentageRight = ((target - differenceRight)/target);
        if(percentageLeft > stop || percentageRight > stop)			   //if the angle is still large, turn fast
        {
            speedLeft = speed * leftSign;
            speedRight = speed * rightSign;
        }
        else if(percentageLeft < -stop || percentageRight < -stop)
        {
            speedLeft = speed * rightSign;
            speedRight = speed * leftSign;
        }
        else														  //if it approaches the end of angle, travel
        {															  //at propertional speed to the angle left
            speedLeft = (percentageLeft*speed*leftSign) + leftSign;
            speedRight = (percentageRight*speed*rightSign) + rightSign;
            if((speedLeft > -1 && speedLeft < 1) && (speedRight > -1 && speedRight < 1) && turning)
            {
                corrected = true;
                speedLeft = rightSign;
                speedRight = leftSign;
            }
        }
        if(turning)
            printf("T\t%i\t%i\t%f\t%f\n", differenceLeft, differenceRight, speedLeft, speedRight);  //T for turn
        else
            printf("L\t%i\t%i\t%f\t%f\n", differenceLeft, differenceRight, speedLeft, speedRight);
        
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
}

void turn (char direction, float angle, float speed)
{
    int *left = malloc(sizeof(int));
    int *right = malloc(sizeof(int));
    get_motor_encoders(left, right);
    int initialLeft = *left;
    int initialRight = *right;
    float ratio = 2.37;							//represents a 1 degree turn in terms of the encoder
    float encoder = ratio*angle;
    if (direction == 'L')
        turningProcess(initialLeft, initialRight, encoder, -1, 1, speed, true);
    else if (direction == 'R')
        turningProcess(initialLeft, initialRight, encoder, 1, -1, speed, true);
    free(left);
    free(right);
}

void straightLine(float distance, float speed)
{
    int *left = malloc(sizeof(int));
    int *right = malloc(sizeof(int));
    get_motor_encoders(left, right);
    int initialLeft = *left;
    int initialRight = *right;
    float targetDistance = 1194 * distance;     // 1 m is 1194 clicks
    int forward = (int)(distance/abs(distance));
    
    turningProcess(initialLeft, initialRight, targetDistance, forward, forward, speed, false);
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
    distance = 1.0;
    speed1 = 50;
    speed2 = 127;
    direction = 'L';
    angle = 90;
    turn('L', 180, 127);
    
    while(i < 4)
    {
        usleep(40000);
        straightLine(1, speed1);
        turn(direction, angle, speed2);
        i++;
    }
}