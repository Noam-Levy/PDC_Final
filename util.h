#include "point.h"

#define STR_MAX 255
#define MIN_CRITERIA_POINTS 3

Point* readData(char* path, int* N, int* K, double* D, int* tCount);
void setPointsPositions(Point* points, int size, double t);
void calculateDistances(Point* points, int size);
int checkProximityCriteria(Point* points, int size, double minimumDistance, int minimumPoints, double t);
