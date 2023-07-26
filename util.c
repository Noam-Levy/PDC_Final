#include <stdlib.h>
#include <stdio.h>
#include "util.h"

Point* readData(char* path, int* N, int* K, double* D, int* tCount)
{
  FILE* f;
  int i, j;
  f = fopen(path, "r"); // open file in read mode
  if (f == NULL)
  {
    fprintf(stderr, "Cannot open file: %s\n", path);
    return NULL;
  }
  if (fscanf(f, "%d %d %lf %d\n", N, K, D, tCount) != 4)
  {
    fprintf(stderr, "Failed reading metadata from file\n");
    fclose(f);
    return NULL;
  }
  Point* points = (Point*)malloc(*N * sizeof(Point));
  if (!points)
  {
    fprintf(stderr, "Unable to allocate memory for points data\n");
    fclose(f);
    return NULL;
  }
  for (i = 0; i < *N; i++)
  {
    j = fscanf(f, "%d %lf %lf %lf %lf\n", &points[i].id, &points[i].x1, &points[i].x2, &points[i].a, &points[i].b);
    points[i].distances = (double*)malloc(*N * sizeof(double));
    if (j != 5 || !points[i].distances)
    {
      if (j != 5)
        fprintf(stderr, "Unable to read point %d data\n", i);
      else
        fprintf(stderr, "Failed allocating distances array for point %d\n", i);

      for (j = 0; j < i; j++)
        free(points[i].distances);
      free(points);
      fclose(f);
      return NULL;
    }
  }
  fclose(f);
  return points;
}

void setPointsPositions(Point* points, int size, double t)
{
  for (int i = 0; i < size; i++)
    setPosition(&points[i], t);
}

void calculateDistances(Point* points, int size)
{
  for (int i = 0; i < size; i++)
    for (int j = 0; j < size; j++)
    {
      points[i].distances[j] = calculateDistanceBetweenPoints(&points[i], &points[j]);
    }
}

int isPointSatisfiesCriteria(Point* p, int size, double minimumDistance, int minimumPoints)
{
  int count = 0;
  for (int i = 0; i < size; i++)
  {
    double dist = p->distances[i];
    count += dist >= 0 && dist < minimumDistance; // negative distance indicates distance to self
  }
  return count >= minimumPoints;
}

int checkProximityCriteria(Point* points, int size, double minimumDistance, int minimumPoints, double t)
{
  int* pointIds = (int*)malloc(MIN_CRITERIA_POINTS * sizeof(int));
  if (!pointIds)
  {
    fprintf(stderr, "Failed allocating IDs array.\n");
    return -1;
  }
  int criteriaMetCounter = 0;
  for (int i = 0; i < size && criteriaMetCounter < MIN_CRITERIA_POINTS; i++)
    if (isPointSatisfiesCriteria(&points[i], size, minimumDistance, minimumPoints))
      pointIds[criteriaMetCounter++] = points[i].id;

  if (criteriaMetCounter >= MIN_CRITERIA_POINTS)
  {
    printf("Points ");
    for (int i = 0; i < MIN_CRITERIA_POINTS - 1; i++)
      printf("%d, ", pointIds[i]);
    printf("%d satisfy Proximity Criteria at t=%.2f\n", pointIds[MIN_CRITERIA_POINTS - 1], t);
  }
  free(pointIds);
  return criteriaMetCounter;
}
