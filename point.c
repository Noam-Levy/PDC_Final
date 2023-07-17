#include <math.h>
#include <stdlib.h>
#include "point.h"

Point* allocatePoint(int distancesSize)
{
  Point* p = (Point*)malloc(sizeof(Point));
  if (!p)
    return NULL;
  p->distances = (distance_t **)malloc(distancesSize * sizeof(distance_t *));
  if (!p->distances)
  {
    free(p);
    return NULL;
  }
  for (int i = 0; i < distancesSize; i++)
  {
    p->distances[i] = (distance_t *)malloc(sizeof(distance_t));
    if (p->distances[i] == NULL)
    {
      for (int j = 0; j < i; j++)
        free(p->distances[j]);
      free(p->distances);
      return NULL;
    }
  }

  return p;
}

void deallocatePoint(Point* p, int distancesSize)
{
  for (int i = 0; i < distancesSize; i++)
    free(p->distances[i]);
  free(p->distances);
  free(p);
}

void setPosition(Point* p, float t)
{
  p->x = ((p->x2 - p->x1) / 2) * sin(t * M_PI_2) + ((p->x2 + p->x1) / 2);
  p->y = p->a * p->x + p->b;
}

float calculateDistanceBetweenPoints(Point* p1, Point* p2)
{
  if (p1->id == p2->id)
    return 0;

  float xDist = pow((p2->x - p1->x), 2);
  float yDist = pow(p2->y - p1->y, 2);
  return sqrt(xDist + yDist);
}
