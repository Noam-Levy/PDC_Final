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

void calculateTimes(double* timesArr, int low, int high, int tCount)
{
  #pragma omp parallel for
  for (int i = low; i < high; i++)
    timesArr[i - low] = 2.0 * i / tCount - 1;
}

void setPointsPositions(Point* points, int size, double t)
{
  #pragma omp parallel for
  for (int i = 0; i < size; i++)
    setPosition(&points[i], t);
}

void calculateDistances(Point* points, int size)
{
  #pragma omp parallel for
  for (int i = 0; i < size; i++)
    for (int j = 0; j < size; j++)
    {
      points[i].distances[j] = calculateDistanceBetweenPoints(&points[i], &points[j]);
    }
}

int isPointSatisfiesCriteria(Point* p, int size, double minimumDistance, int minimumPoints)
{
  int count = 0;
  #pragma omp parallel for reduction(+: count)
  for (int i = 0; i < size; i++)
  {
    double dist = p->distances[i];
    count += dist >= 0 && dist < minimumDistance; // negative distance indicates distance to self
  }
  return count >= minimumPoints;
}

void checkProximityCriteria(Point* points, int size, double minimumDistance, int minimumPoints, double t, criteria_t* result)
{
  int criteriaMetCounter = 0;
  #pragma omp parallel for shared(criteriaMetCounter)
  for (int i = 0; i < size; i++)
    if (criteriaMetCounter < MIN_CRITERIA_POINTS && isPointSatisfiesCriteria(&points[i], size, minimumDistance, minimumPoints))
    {
      #pragma omp critical
      result->pointIDs[criteriaMetCounter++] = points[i].id;
    }

  result->t = t;
  result->isFound = criteriaMetCounter >= MIN_CRITERIA_POINTS;
}
