#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "omp.h"
#include "util.h"

int readPoints(FILE* file, Point** points, int size)
{
  int i;
  for (i = 0; i < size; ++i)
  {
    Point *p = allocatePoint(size);
    if (!p)
      break;

    fscanf(file, "%d %lf %lf %lf %lf\n", &p->id, &p->x1, &p->x2, &p->a, &p->b);
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
  data->points = (Point**)malloc(data->N * sizeof(Point));
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
      deallocatePoint(data->points[i]);           
    
    free(data->points);
    free(data);
    fclose(f);
    return NULL;
  }

  fclose(f);
  return data;
}

void deallocateMetadata(metadata* data)
{
  #pragma omp parallel for
  for(int i = 0; i < data->N; i++)
    deallocatePoint(data->points[i]);
  free(data->points);
  free(data);
}

void calculateTimes(double* timesArr, int low, int high, int tCount)
{
  #pragma omp parallel for
  for (int i = low; i <= high; i++)
    timesArr[i] = 2.0 * i / tCount - 1;
}

void setPointsPositions(Point** points, int size, float t)
{
  #pragma omp parallel for
  for (int i = 0; i < size; i++)
    setPosition(points[i], t);
}

void calculateDistances(Point** points, int size)
{
  #pragma omp parallel for
  for (int i = 0; i < size; i++)
  {
    for (int j = 0; j < size; j++)
      points[i]->distances[j] = calculateDistanceBetweenPoints(points[i], points[j]);
  }
}

int isPointSatisfiesCriteria(Point* p, int size, float minimumDistance, int minimumPoints)
{
  int count = 0;
  #pragma omp parallel for reduction(+: count)
  for (int i = 0; i < size; i++)
  {
    double d = p->distances[i];
    count += d >= 0 && d < minimumDistance; // negative distance indicates distance to self
  }
  return count >= minimumPoints;
}

void checkProximityCriteria(Point **points, int size, float minimumDistance, int minimumPoints, float t, criteria_t* result)
{
  int criteriaMetCounter = 0;
  #pragma omp parallel for shared(criteriaMetCounter)
  for (int i = 0; i < size; i++)
  {
    if (criteriaMetCounter < MIN_CRITERIA_POINTS && isPointSatisfiesCriteria(points[i], size, minimumDistance, minimumPoints))
    {
      #pragma omp critical
      result->pointIDs[criteriaMetCounter++] = points[i]->id;      
    }
  }
  result->t = t;
  result->isFound = criteriaMetCounter >= MIN_CRITERIA_POINTS;
}

MPI_Datatype defineMPIDatatype(void* structPtr, int blocklen[], MPI_Datatype blockTypes[], int numElements)
{
  MPI_Datatype newType;
  MPI_Aint* displacements = (MPI_Aint*)malloc(numElements * sizeof(MPI_Aint));

  // calculate displacements of each member in the struct and adjust relative to base address
  #pragma omp parallel for
  for (int i = 0; i < numElements; i++)
  {
    MPI_Get_address((char*)structPtr + i * sizeof(MPI_Aint), &displacements[i]);
    displacements[i] -= (MPI_Aint)structPtr;
  }

  // create struct and commit datatype
  MPI_Type_create_struct(numElements, blocklen, displacements, blockTypes, &newType);
  MPI_Type_commit(&newType);
  
  free(displacements);
  return newType;
}

