//
//  main.c
//  Task1
//
//  Created by Henry Mortimer on 14/01/2015.
//  Copyright (c) 2015 Henry Mortimer. All rights reserved.
//

#include <stdio.h>
#include "picomms.h"

void task1()
{
    int speed = 100;
    for(int i = 0; i<297; i++)
    {
        set_motor(LEFT, speed);
    }
}

void task2()
{
    int speed = 100;
    for(int i=0; i<100; i++)
    {
        set_motors(speed, speed);
    }
}

void rightTurn()
{
    for (int i = 0; i<74; i++)
    {
        set_motors(100,0);
    }
}

void task3()
{
    for (int i = 0; i<4; i++)
    {
        task2();
        rightTurn();
    }
}


int main()
{
    connect_to_robot();
    initialize_robot();
    task3();
}