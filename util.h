#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <omp.h>

#define STR_MAX 255
#define MIN_CRITERIA_POINTS 3
#define MASTER 0
#define TAG 1

typedef struct {
  double t;
  int isFound;
  int pointIDs[MIN_CRITERIA_POINTS];
} criteria_t;

typedef struct
{
  int id;
  double x1, x2, a, b, x, y;
} Point;

Point* readData(char* path, int* N, int* K, double* D, int* tCount);
void calculateTimes(double *timesArr, int low, int high, int tCount);
void computeProximities(Point *h_points, int size, double* h_times, criteria_t *h_results, int chunk, int tCount, double minimumDistance, int minimumPoints);
