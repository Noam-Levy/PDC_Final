#include <stdlib.h>
#include <stdio.h>
#include "util.h"

int readPoints(FILE *file, Point **points, int size)
{
  int i = 0;
  for (i = 0; i < size; ++i)
  {
    Point *p = (Point *)malloc(sizeof(Point));
    if (!p)
    {
      return i;
    }
    fscanf(file, "%d %f %f %f %f\n", &p->id, &p->x1, &p->x2, &p->a, &p->b);
    points[i] = p;
  }
  return i;
}

metadata *readData(char *path)
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
      free(data->points[i]);  
    free(data->points);
    free(data);
    fclose(f);
    return NULL;
  }

  fclose(f);
  return data;
}
