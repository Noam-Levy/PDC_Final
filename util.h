#include "point.h"

#define MIN_CRITERIA_POINTS 3

typedef struct {
  int N;        // number of points in the set
  int K;        // minimal number of points to satisfy the proximity criteria
  int tCount;   // time interval
  float D;      // maximum distance between points to satisfy criteria
  Point** points;
} metadata;


metadata* readData(char* path);
void deallocateMetadata(metadata* data);
void setPointsPositions(Point** points, int size, float t);
void calculateDistances(Point** points, int size);
int checkProximityCriteria(Point** points, int size, float minimumDistance, int minimumPoints, float t);
