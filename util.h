#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <omp.h>
#include "point.h"

#define STR_MAX 255
#define MIN_CRITERIA_POINTS 3
#define MASTER 0
#define TAG 1

typedef struct {
  double t;
  int isFound;
  int pointIDs[MIN_CRITERIA_POINTS];
} criteria_t;

Point* readData(char* path, int* N, int* K, double* D, int* tCount);
void calculateTimes(double* timesArr, int low, int high, int tCount);
void setPointsPositions(Point* points, int size, double t);
void calculateDistances(Point* points, int size);
void checkProximityCriteria(Point* points, int size, double minimumDistance, int minimumPoints, double t, criteria_t* result);
