#include "phase2.h"

int decideDirectionInt(Mapping *mapping, Point *points[16], bool walls[16][16], int address, int orientation);
int decideDirection1(Mapping *mapping, Point *points[16], bool walls[16][16], int address, int orientation);
int decideDirection2(Mapping *mapping, Point *points[16], bool walls[16][16], int address, int orientation);