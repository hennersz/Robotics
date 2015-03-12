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
int checkOrientation(Mapping *mapping);
double toDegrees(double angle);
void calculateRatio();
