typedef struct
{
  /*
    Describes the distance to point id (from the point which holds the struct).
    the struct is needed because there is no guarantee for size - 1 consecutive ids. 
  */
  int id;
  float distance;
} distance_t;

typedef struct
{
  int id;
  float x1, x2, a, b, x, y;
  distance_t** distances;
} Point;

typedef struct {
  int N;        // number of points in the set
  int K;        // minimal number of points to satisfy the proximity criteria
  int tCount;   // time interval
  float D;      // maximum distance between points to satisfy criteria
  Point** points;
} metadata;

#define MIN_CRITERIA_POINTS 3

metadata* readData(char* path);
void setPointsPositions(Point** points, int size, float t);
void calculateDistances(Point** points, int size);
void checkProximityCriteria(Point** points, int size, float minimumDistance, int minimumPoints, float t);
