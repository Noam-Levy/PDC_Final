#include "mpi.h"
#include "point.h"

#define MIN_CRITERIA_POINTS 3
#define MASTER 0
#define TAG 1
#define STOP_TAG 2

typedef struct {
  int N;        // number of points in the set
  int K;        // minimal number of points to satisfy the proximity criteria
  int tCount;   // time interval
  float D;      // maximum distance between points to satisfy criteria
  Point** points;
} metadata;

metadata* readData(char *path);
void deallocateMetadata(metadata* data);
void calculateTimes(double* timesArr, int low, int high, int tCount);

void setPointsPositions(Point** points, int size, float t);
void calculateDistances(Point** points, int size);
int checkProximityCriteria(Point** points, int size, float minimumDistance, int minimumPoints, float t);

MPI_Datatype defineMPIDatatype(void* structPtr, int blocklen[], MPI_Datatype blockTypes[], int numElements);

