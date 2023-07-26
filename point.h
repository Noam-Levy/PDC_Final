#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef struct
{
  int id;
  double x1, x2, a, b, x, y;
  double* distances;
} Point;

void setPosition(Point* p, double t);
double calculateDistanceBetweenPoints(Point* p1, Point* p2);
