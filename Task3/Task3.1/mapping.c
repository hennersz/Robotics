#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "picomms.h"

#define WIDTH 260
#define WHEELDIAM 96

float ratio = 0.42;

void calculateRatio()
{
	float wheelCirc = WHEELDIAM * M_PI;
	float robotCirc = WIDTH * M_PI;
	ratio = 1/(robotCirc/wheelCirc);
}

int findAngle(int leftEncoder, int rightEncoder, float ratio)
{
	float temp;
	temp = (leftEncoder - rightEncoder)/2.0 * ratio;
	return (int)temp % 360;
}

double clicksToMM(int clicks)
{
	double wheelCirc = WHEELDIAM * M_PI;
	return clicks*wheelCirc/360.0;
}

double toRadians(double angle)
{
	return (double)(angle * (M_PI/180));
}

void encoderChange(int* previousLeft, int* previousRight, int* deltaL, int* deltaR)
{
	int lEnc, rEnc;
	get_motor_encoders(&lEnc, &rEnc);
	*deltaL = lEnc - *previousLeft;
	*deltaR = rEnc - *previousRight;
	*previousLeft = lEnc;
	*previousRight = rEnc;
	
}

void straightDistance(int distance, float* x, float* y)
{
	int lEnc, rEnc;
	get_motor_encoders(&lEnc, &rEnc);
	double degrees = (double)findAngle(lEnc, rEnc, ratio);
	double radians = toRadians(degrees);
	distance = clicksToMM(distance);
	*y += (distance * (cos(radians)));
	*x += (distance * (sin(radians))); 
}

double angleChange(int deltaL, int deltaR)
{
	double dl = clicksToMM(deltaL);
	double dr = clicksToMM(deltaR);
	double angle = (dl-dr)/WIDTH;
	return angle;
}

void positionChange(double* previousAngle, int deltaL, int deltaR, float* x, float* y)
{
	double currentAngle = angleChange(deltaL, deltaR); 
	double dL = clicksToMM(deltaL);
	double dR = clicksToMM(deltaR);
	double rL = dL/currentAngle;
	double rR = dR/currentAngle;
	double rM = (rL+rR)/2.0;

	if(*previousAngle == 0)
	{
		*x += rM - rM * cos(currentAngle); printf("X += %f\n",rM - rM * cos(currentAngle));
		*y += rM * sin(currentAngle); printf("Y += %f\n",rM * sin(currentAngle));
	}
	else
	{
		*x += rM * cos(*previousAngle + currentAngle) - rM * cos(*previousAngle); printf("X += %f\n",rM * cos(*previousAngle + currentAngle) - rM * cos(*previousAngle));
		*y += rM * sin(*previousAngle + currentAngle) - rM * sin(*previousAngle); printf("Y += %f\n",rM * sin(*previousAngle + currentAngle) - rM * sin(*previousAngle));
	}
	*previousAngle += currentAngle;
}

void distanceTravelled(double* previousAngle, float* x, float* y, int* previousLeft, int* previousRight)
{
	int deltaL, deltaR;
	encoderChange(previousLeft, previousRight, &deltaL, &deltaR);
	if(deltaL == deltaR)
	{
		straightDistance(deltaL, x, y);
	}
	else
	{
		positionChange(previousAngle, deltaL, deltaR, x, y);
	}
	printf("X:%.0f MM , Y: %.0f MM\n", *x, *y);
}