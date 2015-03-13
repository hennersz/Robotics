#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "picomms.h"
#include "mapping.h"

#define WIDTH 225
#define WHEELDIAM 96

double ratio;

void initialiseMapping(Mapping *mapping)
{
	mapping->previousAngle = 0;
	mapping->previousLeft = 0;
	mapping->previousRight = 0;
	mapping->x = 0;
	mapping->y = 0;
}

void calculateRatio()
{
	double wheelCirc = WHEELDIAM * M_PI;
	double robotCirc = WIDTH * M_PI;
	ratio = 1/(robotCirc/wheelCirc);
}
/*
double findAngle(Mapping mapping)
{
	double angle;
	angle = (mapping->previousLeft - mapping->previousRight)/2.0 * ratio;
	return angle;
}
*/
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

void straightDistance(int distance, Mapping *m)
{
	
	double angle = m->previousAngle;
	//printf("degrees = %f\n", degrees);
	distance = clicksToMM(distance);
	m->y += (distance * (cos(angle)));
	m->x += (distance * (sin(angle))); 
}

double angleChange(int deltaL, int deltaR)
{
	double dl = clicksToMM(deltaL);
	double dr = clicksToMM(deltaR);
	double angle = (dl-dr)/WIDTH;
	return angle;
}

void positionChange(Mapping *m, int deltaL, int deltaR)
{
	double currentAngle = angleChange(deltaL, deltaR); 
	double dL = clicksToMM(deltaL);
	double dR = clicksToMM(deltaR);
	double rL = dL/currentAngle;
	double rR = dR/currentAngle;
	double rM = (rL+rR)/2.0;

	if(m->previousAngle == 0)
	{
		m->x -= rM - rM * cos(currentAngle);
		m->y += rM * sin(currentAngle); 
	}
	else
	{
		m->x -= rM * cos(m->previousAngle + currentAngle) - rM * cos(m->previousAngle);
		m->y += rM * sin(m->previousAngle + currentAngle) - rM * sin(m->previousAngle);
	}
	m->previousAngle += currentAngle;
}

void distanceTravelled(Mapping *mapping)
{
	int deltaL, deltaR;
	encoderChange(&mapping->previousLeft, &mapping->previousRight, &deltaL, &deltaR);
	if(deltaL == deltaR)
	{
		straightDistance((deltaL+deltaR)/2, mapping);
	}
	else
	{
		positionChange(mapping, deltaL, deltaR);
	}
	//set_point((mapping->x)/10, (mapping->y/10));
	//double distance = sqrt((mapping->x)*(mapping->x) + (mapping->y)*(mapping->y));
	//double angle = atan((mapping->x)/(mapping->y));
	//angle *= (180/M_PI);
	//printf("Distance = %f\tangle = %f\n", distance, angle);
}