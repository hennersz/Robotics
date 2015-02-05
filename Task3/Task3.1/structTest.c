#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "picomms.h"

struct encoders
{
	int left;
	int right;
};

void pointersTest(struct encoders *encoders)
{
	get_motor_encoders(&encoders->left, &encoders->right);
	printf("left value: %i, right value: %i \n",*encoders->left, *encoders->right);
}
int main()
{
	struct encoders encoders;
	connect_to_robot();
	initialize_robot();
	while(1)
	{
		set_motors(10,10);
		get_motor_encoders(&encoders.left, &encoders.right);
		printf("left value: %i, right value: %i \n",encoders.left, encoders.right);
	}
	return 0;
}