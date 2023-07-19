typedef struct
{
  int id;
  double x1, x2, a, b, x, y;
  double* distances;
} Point;

Point *allocatePoint(int distancesSize);
void deallocatePoint(Point* p);
void setPosition(Point* p, float t);
float calculateDistanceBetweenPoints(Point* p1, Point* p2);

