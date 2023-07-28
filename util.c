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
    if (j != 5)
    {    
      fprintf(stderr, "Unable to read point %d data\n", i);
      free(points);
      fclose(f);
      return NULL;
    }
  }

  fclose(f);
  return points;
}

void calculateTimes(double *timesArr, int low, int high, int tCount)
{
  #pragma omp parallel for
  for (int i = low; i < high; i++)
      timesArr[i - low] = 2.0 * i / tCount - 1;
}

