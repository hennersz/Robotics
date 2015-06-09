#define WIDTH 246     //Not sure if these are lily's but I think they are : 233 and 95
#define WHEELDIAM 103

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
void set_motors1(int speed_l, int speed_r);
void distanceTravelled(Mapping *mapping);
void initialiseMapping(Mapping *mapping);
int checkOrientation(Mapping *mapping);
double toDegrees(double angle);
double toRadians(double angle);
void set_origin2(int x, int y);
void set_point1(int x, int y);
int addressToX(int address);
int addressToY(int address);
int connect_to_simulator();
void calculateRatio();
void set_origin1();
