#include <stdlib.h>
#include <stdio.h>
#include "util.h"

int readPoints(FILE* file, Point** points, int size)
{
  int i;
  for (i = 0; i < size; ++i)
  {
    Point *p = allocatePoint(size);
    if (!p)
      break;

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
      deallocatePoint(data->points[i], data->N);           
    
    free(data->points);
    free(data);
    fclose(f);
    return NULL;
  }

  fclose(f);
  return data;
}

void deallocateMetadata(metadata *data)
{
  for(int i = 0; i < data->N; i++)
    deallocatePoint(data->points[i], data->N);
  free(data->points);
  free(data);
}

void setPointsPositions(Point** points, int size, float t)
{
  for (int i = 0; i < size; i++)
    setPosition(points[i], t);
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

int isPointSatisfiesCriteria(Point* p, int size, float minimumDistance, int minimumPoints)
{
  int count = 0;
  for (int i = 0; i < size; i++)
    count += p->distances[i]->distance < minimumDistance;
  return count >= minimumPoints;
}

int checkProximityCriteria(Point** points, int size, float minimumDistance, int minimumPoints, float t)
{
  int* pointIds = (int*)malloc(MIN_CRITERIA_POINTS * sizeof(int));
  if (!pointIds)
  {
    fprintf(stderr, "Failed allocating IDs array.\n");
    return -1;
  }
  int criteriaMetCounter = 0;
  for (int i = 0; i < size; i++)
    if (isPointSatisfiesCriteria(points[i], size, minimumDistance, minimumPoints))
      pointIds[criteriaMetCounter++] = points[i]->id;

  if (criteriaMetCounter >= MIN_CRITERIA_POINTS)
  {
    printf("Points ");
    for (int i = 0; i < MIN_CRITERIA_POINTS - 1; i++)
      printf("%d, ", pointIds[i]);
    printf("%d satisfy proximity criteria at t=%.2f\n", pointIds[MIN_CRITERIA_POINTS - 1], t);
  }
  free(pointIds);
  return criteriaMetCounter;
}
