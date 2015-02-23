typedef struct Mapping 
{
	double previousAngle;
	float x;
	float y;
	int previousLeft;
	int previousRight;
}Mapping;

float ratio;
//int findAngle(int leftEncoder, int rightEncoder);
void distanceTravelled(Mapping *mapping);
void initialiseMapping(Mapping *mapping);
void calculateRatio();
