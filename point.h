typedef struct
{
  int id;
  float x1, x2, a, b, x, y;
  float* distances;
} Point;

Point *allocatePoint(int distancesSize);
void deallocatePoint(Point* p);
void setPosition(Point* p, float t);
float calculateDistanceBetweenPoints(Point* p1, Point* p2);
