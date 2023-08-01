#include <cuda_runtime.h>
#include "util.h"

#define THREADS_PER_BLOCK 256

__device__ int criteriaMetCounter = 0;

void checkError(cudaError_t err, int lineNum)
{
  if (err != cudaSuccess)
  {
    fprintf(stderr, "CUDA Error on line %d: %s\n", lineNum, cudaGetErrorString(err));
    exit(EXIT_FAILURE);
  }
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

__device__ void resetCriteriaMetCounter()
{
  criteriaMetCounter = 0;
}

__global__ void setResultMetadata(criteria_t* res, double t)
{
  res->t = t;
  res->isFound = criteriaMetCounter >= MIN_CRITERIA_POINTS;
  resetCriteriaMetCounter();
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
  if (tid > size || atomicAdd(&criteriaMetCounter, 0)  >= MIN_CRITERIA_POINTS)
    return;
  
  if (isPointSatisfiesCriteria(&points[tid], points, size, minimumDistance, minimumPoints))
  {
    int counter = atomicAdd(&criteriaMetCounter, 1); // counter recieves the previous value of criteriaMetCounter.
    if (counter < MIN_CRITERIA_POINTS)
      result->pointIDs[counter] = points[tid].id;
  }
}

void computeProximities(Point *h_points, int size, double* h_times, criteria_t *h_results, int chunk, double minimumDistance, int minimumPoints)
{
  cudaError_t err;
  int i, blocks;

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
  
  blocks = (int)floor((size * chunk) / THREADS_PER_BLOCK) + 1; // +1 handles case where size * chunk < THREADS_PER_BLOCK

  for (i = 0; i < chunk; i++)
  {
    double t = h_times[i];
    
    setPointsPositions<<<blocks, THREADS_PER_BLOCK>>>(d_points, size, t);
    checkError(cudaGetLastError(), __LINE__ - 1);
    
    checkProximityCriteria<<<blocks, THREADS_PER_BLOCK>>>(d_points, size, t, minimumDistance, minimumPoints, &d_results[i]);
    checkError(cudaGetLastError(), __LINE__ - 1);
    
    setResultMetadata<<<1, 1>>>(&d_results[i], t);
    checkError(cudaGetLastError(), __LINE__ - 1);
    cudaDeviceSynchronize(); // wait for all threads to finish with current timestamp
  }
  // copy results to host
  err = cudaMemcpy(h_results, d_results, chunk * sizeof(criteria_t), cudaMemcpyDeviceToHost);
  checkError(err, __LINE__ - 1);

  // free allocated device memory
  cudaFree(d_points);
  cudaFree(d_results);
}
