#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "util.h"

int readPoints(FILE* file, Point** points, int size)
{
  int i;
  for (i = 0; i < size; ++i)
  {
    Point *p = (Point*)malloc(sizeof(Point));
    if (!p)
      break;
    
    p->distances = (distance_t**)malloc(size * sizeof(distance_t*));
    if (!p->distances)
      break;
    for (int j = 0; j < size; j++)
    {
      p->distances[j] = (distance_t*)malloc(sizeof(distance_t));
      if (p->distances[j] == NULL)
      {
        for (int k = 0; k < j; k++)
          free(points[j]->distances[k]);
        free(points[j]->distances);
        break;
      }
    }

    fscanf(file, "%d %f %f %f %f\n", &p->id, &p->x1, &p->x2, &p->a, &p->b);
    points[i] = p;
  }
  return i;
}

metadata* readData(char* path)
{
  FILE* f;
  f = fopen(path, "r"); // open file in read mode
  if (f == NULL)
  {
    fprintf(stderr, "Cannot open file: %s\n", path);
    return NULL;
  }
  metadata* data = (metadata*)malloc(sizeof(metadata));
  if (data == NULL)
  {
    fprintf(stderr, "Unable to allocate memory for metadata\n");
    fclose(f);
    return NULL;
  }

  fscanf(f, "%d %d %f %d\n", &data->N, &data->K, &data->D, &data->tCount);
  data->points = (Point**)malloc(data->N * sizeof(Point*));
  if (data->points == NULL)
  {
    fprintf(stderr, "Unable to allocate memory for points data\n");
    free(data);
    fclose(f);
    return NULL;
  }
  int allocatedPoints = readPoints(f, data->points, data->N);
  if (allocatedPoints != data->N)
  {
    fprintf(stderr, "Unable to allocate memory for points data\n");
    for (int i = 0; i < allocatedPoints; i++)
    {
      // deallocate distances array
      for (int j = 0; j < data->N; j++)
        free(data->points[i]->distances[j]);
      free(data->points[i]->distances);
      // deallocate point
      free(data->points[i]);            
    }
    free(data->points);
    free(data);
    fclose(f);
    return NULL;
  }

  fclose(f);
  return data;
}

void setPointsPositions(Point** points, int size, float t)
{
  for (int i = 0; i < size; i++)
  {
    Point* p = points[i];
    p->x = ((p->x2 - p->x1) / 2) * sin(t * M_PI_2) + ((p->x2 + p->x1) / 2);
    p->y = p->a * p->x + p->b;
  }
}

float calculateDistanceBetweenPoints(Point* p1, Point* p2)
{
  if (p1->id == p2->id)
    return 0;

  float xDist = pow((p2->x - p1->x), 2);
  float yDist = pow(p2->y - p1->y, 2);
  return sqrt(xDist + yDist);
}

void calculateDistances(Point** points, int size)
{
  for (int i = 0; i < size; i++)
    for (int j = 0; j < size; j++)
    {
      points[i]->distances[j]->id = points[j]->id;
      points[i]->distances[j]->distance = calculateDistanceBetweenPoints(points[i], points[j]);
    }
}

void checkProximityCriteria(Point** points, int size, float minimumDistance, int minimumPoints, float t)
{
  int *pointIds = (int *)malloc(MIN_CRITERIA_POINTS * sizeof(int));
  if (!pointIds)
  {
    fprintf(stderr, "Failed allocating IDs array.\n");
    return;
  }
  int criteriaMetCounter = 0;
  for (int i = 0; i < size && criteriaMetCounter < MIN_CRITERIA_POINTS; i++)
  {
    int timesPointMetCriteria = 0;
    distance_t** localDistances = points[i]->distances;
    
    for (int j = 0; j < size && timesPointMetCriteria < minimumPoints; j++)
    {
      if (i != j && localDistances[j]->distance >= minimumDistance)
        timesPointMetCriteria++;
    }
    
    if (timesPointMetCriteria >= minimumPoints)
      pointIds[criteriaMetCounter++] = points[i]->id;
  }

  if (criteriaMetCounter < MIN_CRITERIA_POINTS)
    printf("There are no %d points found for t=%.2f\n", MIN_CRITERIA_POINTS, t);
  else
  {
    printf("Points ");
    for (int i = 0; i < MIN_CRITERIA_POINTS - 1; i++)
      printf("%d, ", pointIds[i]);
    printf("%d satisfy proximity criteria at t=%.2f\n", pointIds[MIN_CRITERIA_POINTS - 1], t);
  }
}
