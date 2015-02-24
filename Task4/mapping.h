typedef struct Mapping 
{
	double previousAngle;
	double x;
	double y;
	int previousLeft;
	int previousRight;
}Mapping;

double ratio;
//int findAngle(int leftEncoder, int rightEncoder);
void distanceTravelled(Mapping *mapping);
void initialiseMapping(Mapping *mapping);
void calculateRatio();
