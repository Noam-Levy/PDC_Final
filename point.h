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
  distance_t **distances;
} Point;

Point *allocatePoint(int distancesSize);
void deallocatePoint(Point* p, int distancesSize);
void setPosition(Point* p, float t);
float calculateDistanceBetweenPoints(Point* p1, Point* p2);
