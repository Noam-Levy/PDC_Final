#include <math.h>
#include <stdlib.h>
#include "point.h"

void setPosition(Point* p, double t)
{
  p->x = ((p->x2 - p->x1) / 2) * sin(t * M_PI_2) + ((p->x2 + p->x1) / 2);
  p->y = p->a * p->x + p->b;
}

double calculateDistanceBetweenPoints(Point* p1, Point* p2)
{
  if (p1->id == p2->id)
    return -1;

  float xDist = pow((p2->x - p1->x), 2);
  float yDist = pow(p2->y - p1->y, 2);
  return sqrt(xDist + yDist);
}
