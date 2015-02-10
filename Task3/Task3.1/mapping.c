#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "picomms.h"

#define WIDTH 210
#define WHEELDIAM 96

float calculateRatio()
{
	float wheelCirc = WHEELDIAM * M_PI;
	float robotCirc = WIDTH * M_PI;
	ratio = 1/(robotCirc/wheelCirc);
	return ratio;
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
<<<<<<< HEAD
	return wheelCirc/360.0;
=======
	return clicks*(wheelCirc/360.0);
>>>>>>> FETCH_HEAD
}

double toRadians(double angle)
{
	return (double)(angle * (M_PI/180));
}

void encoderChange(int* lEnc, int* rEnc, int* deltaL, int* deltaR)
{
	int previousLeft = *lEnc;
	int previousRight = *rEnc;
	get_motor_encoders(lEnc, rEnc);
	*deltaL = *lEnc - previousLeft;
	*deltaR = *rEnc - previousRight;
}

void straightDistance(int distance, int* x, int* y)
{
	int lEnc, rEnc;
	float ratio = calculateRatio();
	get_motor_encoders(&lEnc, &rEnc);
	double degrees = (double)findAngle(lEnc, rEnc, ratio);
	double radians = toRadians(degrees);

	*y += clicksToCM(distance * (cos(radians)));
	*x += clicksToCM(distance * (sin(radians))); 
}

double angleChange(int deltaL, int deltaR)
{
	double dl = clicksToMM(deltaL);
	double dr = clicksToMM(deltaR);
	double angle = (dl-dr)/WIDTH; printf("Angle: %f\n", angle);
	return angle;
}

void positionChange(double* previousAngle, int deltaL, int deltaR, int* x, int* y)
{
	double currentAngle = angleChange(deltaL, deltaR);
	double dL = clicksToMM(deltaL);
	double dR = clicksToMM(deltaR);
	double rL = dL/currentAngle;
	double rR = dR/currentAngle;
	double rM = (rL+rR)/2;

<<<<<<< HEAD
	*x += rM * cos(*previousAngle + currentAngle) - rM * cos(*previousAngle);
	*y += rM * sin(*previousAngle + currentAngle) - rM * sin(*previousAngle);
=======
	if(*previousAngle == 0)
	{
		*x -= rM - rM * cos(currentAngle); printf("X += %f\n",rM - rM * cos(currentAngle));
		*y += rM * sin(currentAngle); printf("Y += %f\n",rM * sin(currentAngle));
	}
	else
	{
		*x -= rM * cos(*previousAngle + currentAngle) - rM * cos(*previousAngle); printf("X += %f\n",rM * cos(*previousAngle + currentAngle) - rM * cos(*previousAngle));
		*y += rM * sin(*previousAngle + currentAngle) - rM * sin(*previousAngle); printf("Y += %f\n",rM * sin(*previousAngle + currentAngle) - rM * sin(*previousAngle));
	}
>>>>>>> FETCH_HEAD
	*previousAngle += currentAngle;
}

void distanceTravelled(double* previousAngle, int* x, int* y)
{
	int lEnc, rEnc, deltaL, deltaR;
	get_motor_encoders(&lEnc, &rEnc);
	encoderChange(&lEnc, &rEnc, &deltaL, &deltaR);
	if(deltaL == deltaR)
	{
		straightDistance(deltaL, x, y);
	}
	else
	{
		positionChange(previousAngle, deltaL, deltaR, x, y);
	}
<<<<<<< HEAD
=======
	set_point(*x/10, *y/10);
	printf("X:%.0f MM , Y: %.0f MM\n", *x, *y);
>>>>>>> FETCH_HEAD
}