#include <cuda_runtime.h>
#include "util.h"

#define THREADS_PER_BLOCK 256

__device__ int lock = 0;
__device__ int criteriaMetCounter = 0;

void checkError(cudaError_t err, int lineNum)
{
  if (err != cudaSuccess)
  {
    fprintf(stderr, "CUDA Error on line %d: %s\n", lineNum, cudaGetErrorString(err));
    exit(EXIT_FAILURE);
  }
}

__device__ void aquireLock()
{
  while (atomicCAS(&lock, 0, 1) != 0);
}

__device__ void releaseLock()
{
  atomicExch(&lock, 0);
}

__device__ double calculateDistanceBetweenPoints(Point* p1, Point* p2)
{
  if (p1->id == p2->id)
    return -1;
  
  float xDist = pow((p2->x - p1->x), 2);
  float yDist = pow(p2->y - p1->y, 2);
  return sqrt(xDist + yDist);
}


__device__ int isPointSatisfiesCriteria(Point* ref, Point* points, int size, double minimumDistance, int minimumPoints)
{
  int count = 0;
  for (int i = 0; i < size; i++)
  {
    if (points[i].id != ref->id)
    {
      double dist = calculateDistanceBetweenPoints(ref, &points[i]);
      count += dist >= 0 && dist < minimumDistance; // negative distance indicates distance to self
    }    
  }
  return count >= minimumPoints;
}

__global__ void setCriteriaMetCounter(int value)
{
  criteriaMetCounter = value;
}

__global__ void setPointsPositions(Point *points, int size, double t)
{
  int threadId = blockIdx.x * blockDim.x + threadIdx.x;
  if (threadId > size)
    return;

  Point p = points[threadId];
  p.x = ((p.x2 - p.x1) / 2) * sin(t * M_PI_2) + ((p.x2 + p.x1) / 2);
  p.y = p.a * p.x + p.b;
}

__global__ void checkProximityCriteria(Point *points, int size, double t, double minimumDistance, int minimumPoints, criteria_t* result)
{
  int tid = blockIdx.x * blockDim.x + threadIdx.x;
  if (tid > size)
    return;
  
  if (isPointSatisfiesCriteria(&points[tid], points, size, minimumDistance, minimumPoints))
  {
    // aquireLock();
    if(criteriaMetCounter < MIN_CRITERIA_POINTS)
    {
      result->pointIDs[criteriaMetCounter++] = points[tid].id;
      printf("%d satisfies criteria at %.2f. counter=%d\n", points[tid].id, t, criteriaMetCounter);
    }
    // releaseLock();
  }

  if (criteriaMetCounter >= MIN_CRITERIA_POINTS)
  {
    // aquireLock();
    if (!result->isFound)
    {
      result->isFound = 1;
      result->t = t;
    }
    // releaseLock();
  }
}

__global__ void printPoints(Point* points, int size)
{
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  if (i > size)
    return;
  
  printf("P[%d]: x=%.2f y=%.2f\n", points[i].id, points[i].x, points[i].y);
}

__global__ void printResults(criteria_t *results, int size)
{
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  if (i > size)
    return;

  criteria_t res = results[i];
  if (res.isFound)
  {
    printf("Points ");
    for (int j = 0; j < MIN_CRITERIA_POINTS - 1; j++)
      printf("%d, ", res.pointIDs[j]);
    printf("%d satisfy Proximity Criteria at t=%.2lf\n", res.pointIDs[MIN_CRITERIA_POINTS - 1], res.t);
  }
  else
  {
    printf("No joy at t=%.2f\n", res.t);
  }
}

void computeProximities(Point *h_points, int size, double* h_times, criteria_t *h_results, int chunk, int tCount, double minimumDistance, int minimumPoints)
{
  cudaError_t err;
  int i, offset;

  // allocate device memory
  Point *d_points;
  err = cudaMalloc((void **)&d_points, size * sizeof(Point));
  checkError(err, __LINE__ - 1);

  criteria_t *d_results;
  err = cudaMalloc((void **)&d_results, chunk * sizeof(criteria_t));
  checkError(err, __LINE__ - 1);

  // copy data to device
  err = cudaMemcpy(d_points, h_points, size * sizeof(Point), cudaMemcpyHostToDevice);
  checkError(err, __LINE__ - 1);
  
  int blocks = (int)floor((size * chunk) / THREADS_PER_BLOCK) + 1; // +1 handles case where size * chunk < THREADS_PER_BLOCK
  for (i = 0; i < chunk; i++)
  {
    setCriteriaMetCounter<<<1, 1>>>(0);
    double t = h_times[i];
    
    setPointsPositions<<<blocks, THREADS_PER_BLOCK>>>(d_points, size, t);
    checkError(cudaGetLastError(), __LINE__ - 1);
    
    checkProximityCriteria<<<blocks, THREADS_PER_BLOCK>>>(d_points, size, t, minimumDistance, minimumPoints, &d_results[i]);
    checkError(cudaGetLastError(), __LINE__ - 1);
  }
  cudaDeviceSynchronize();

  // copy results to host
  err = cudaMemcpy(h_results, d_results, chunk * sizeof(criteria_t), cudaMemcpyDeviceToHost);
  checkError(err, __LINE__ - 1);

  // free allocated device memory
  cudaFree(d_points);
  cudaFree(d_results);
}
