#include "picomms.h"
#include <stdio.h>

int main() {
	initialize_robot();
	connect_to_robot();
	int answer;
	while(1)
	{
		scanf("%c", &answer);
		if(answer == 'w')
			set_motors(30, 30);
		else if(answer == 'a')
			set_motors(0, 30);
		else if(answer == 'd')
			set_motors(30, 0);
		else if(answer == 's')
			set_motors(-30, -30);
		else
			set_motors(0, 0);
	}
}