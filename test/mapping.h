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
double toRadians(double angle);
int addressToX(int address);
int addressToY(int address);
void calculateRatio();
