#define WIDTH 225     //Not sure if these are lily's but I think they are : 233 and 95
#define WHEELDIAM 95

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
