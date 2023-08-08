#include <cuda_runtime.h>
#include "util.h"

#define THREADS_PER_BLOCK 256

__device__ int criteriaMetCounter = 0;

/**
 * @brief Checks for any error in CUDA operations.
 *        prints error message and stops the program for any CUDA error occured.
 * @param err cudaError_t struct
 * @param lineNum interger represent the line number on which the error occured. used for error finding.
**/
void checkError(cudaError_t err, int lineNum)
{
  if (err != cudaSuccess)
  {
    fprintf(stderr, "CUDA Error on line %d: %s\n", lineNum, cudaGetErrorString(err));
    exit(EXIT_FAILURE);
  }
}

/**
 * @brief Calculates the distance between two points p1 and p2.
 * @param p1 Point 1
 * @param p2 Point 2
 * @return The distance between p1 and p2. -1 if p1 == p2.
**/
__device__ double calculateDistanceBetweenPoints(Point* p1, Point* p2)
{
  if (p1->id == p2->id)
    return -1;
  
  double xDist = pow((p2->x - p1->x), 2);
  double yDist = pow(p2->y - p1->y, 2);
  return sqrt(xDist + yDist);
}

/**
 * @brief Determines if the referenced point satisfies the proximity criteria.
 * @param ref The referenced Point.
 * @param points Array of all points.
 * @param size Size of points array
 * @param maximumDistance Maximum distance allowed between the reference point to other points to satisfiy the criteria.
 * @param minimumPoints Minimum points wihin the distance threshold required around the reference point to satisfiy the criteria.
 * @return 1 if the reference point satisfies the criteria, otherwise 0.
**/
__device__ int isPointSatisfiesCriteria(Point* ref, Point* points, int size, double maximumDistance, int minimumPoints)
{
  int count = 0;
  for (int i = 0; i < size && count < minimumPoints; i++)
  {
    double dist = calculateDistanceBetweenPoints(ref, &points[i]);
    count += dist >= 0 && dist < maximumDistance; // negative distance indicates distance to self
  }
  return count >= minimumPoints;
}

/**
 * @brief Helper device function to atomically reset the global criteria met counter.
**/
__device__ void resetCriteriaMetCounter()
{
  atomicExch(&criteriaMetCounter, 0);
}

/**
 * @brief Helper function to set the isCritetiraMet paramter to a referenced result struct.
 * @param res Refernce to the current result struct.
**/
__global__ void setResultMetadata(criteria_t* res)
{
  res->isCritetiraMet = criteriaMetCounter >= MIN_CRITERIA_POINTS;
  resetCriteriaMetCounter();
}

/**
 * @brief Calculates and sets the points positions for given time t.
 * @param points Array of points
 * @param size Size of points array
 * @param t Current time value
**/
__global__ void setPointsPositions(Point *points, int size, double t)
{
  int threadId = blockIdx.x * blockDim.x + threadIdx.x;
  if (threadId > size)
    return;

  Point* p = &points[threadId];
  p->x = ((p->x2 - p->x1) / 2) * sin(t * M_PI_2) + ((p->x2 + p->x1) / 2);
  p->y = p->a * p->x + p->b;
}

/**
 * @brief Checks which points satisfies the proximity criteria in a given time t. results are saved in the referenced result struct.
 * @param points Array of points
 * @param size Size of points array
 * @param maximumDistance Maximum distance allowed between the reference point to other points to satisfiy the criteria.
 * @param minimumPoints Minimum points wihin the distance threshold required around the reference point to satisfiy the criteria.
 * @param result Refernce to the current result struct
**/
__global__ void checkProximityCriteria(Point *points, int size, double maximumDistance, int minimumPoints, criteria_t* result)
{
  int tid = blockIdx.x * blockDim.x + threadIdx.x;
  if (tid > size|| atomicAdd(&criteriaMetCounter, 0)  >= MIN_CRITERIA_POINTS)
    return;
  
  if (isPointSatisfiesCriteria(&points[tid], points, size, maximumDistance, minimumPoints))
  {
    /* 
      try to claim the right to increment criteriaMetCounter and update the results array.
      this is effectively a spin-lock for the right to update the results array.
    */
    while (atomicAdd(&criteriaMetCounter, 0) < MIN_CRITERIA_POINTS)
    {
      int expectedValue = criteriaMetCounter;
      int newValue = expectedValue + 1;
      int value = atomicCAS(&criteriaMetCounter, expectedValue, newValue);

      if (value == expectedValue && value < MIN_CRITERIA_POINTS)
      {
        result->pointIDs[value] = points[tid].id;
        break;
      }
    }
  }
}

void computeProximities(Point *h_points, int size, criteria_t *h_results, int chunk, double maximumDistance, int minimumPoints)
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

  err = cudaMemcpy(d_results, h_results, chunk * sizeof(criteria_t), cudaMemcpyHostToDevice);
  checkError(err, __LINE__ - 1);
  
  blocks = (int)floor((size * chunk) / THREADS_PER_BLOCK) + 1; // +1 handles case where size * chunk < THREADS_PER_BLOCK

  for (i = 0; i < chunk; i++)
  {
    setPointsPositions<<<blocks, THREADS_PER_BLOCK>>>(d_points, size, h_results[i].t);
    checkError(cudaGetLastError(), __LINE__ - 1);
    
    checkProximityCriteria<<<blocks, THREADS_PER_BLOCK>>>(d_points, size, maximumDistance, minimumPoints, &d_results[i]);
    checkError(cudaGetLastError(), __LINE__ - 1);
    
    setResultMetadata<<<1, 1>>>(&d_results[i]);
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
