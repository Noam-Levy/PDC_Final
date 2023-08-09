#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <omp.h>

#define STR_MAX 255
#define MIN_CRITERIA_POINTS 3
#define MASTER 0
#define TAG 1

/**
 * @brief Represents a result for proximity criteria check in time t.
 * @param t time value.
 * @param isCritetiraMet States if the criteria in time t.
 * @param pointIDs Array of pointIDs that stores the IDs of points satisfied the criteria in the current time.
**/
typedef struct {
  double t;
  int isCritetiraMet;
  int pointIDs[MIN_CRITERIA_POINTS];
} criteria_t;

/**
 * @brief Represents a point in a 2D space.
 * @param id Point identifier.
 * @param x1 Constant needed to calculate the X position of the point.
 * @param x2 Constant needed to calculate the X position of the point.
 * @param a Constant needed to calculate the Y position of the point.
 * @param b Constant needed to calculate the Y position of the point.
 * @param x X coordinate of the point in a certain time t.
 * @param y Y coordinate of the point in a certain time t.
**/
typedef struct
{
  int id;
  double x1, x2, a, b, x, y;
} Point;

/**
 * @brief Reads data from the input file.
 * @param path Path to input file.
 * @param N Pointer to integer represents the number of points in the set.
 * @param K Pointer to integer represents the minimal number of points needed to satisfy the proximity criteria.
 * @param D Pointer to double represents the maximum distance allowed between points.
 * @param tCount Pointer to integer represents the time intervals required.
 * @returns A dynamically allocated points array.
**/
Point* readData(char* path, int* N, int* K, double* D, int* tCount);

/**
 * @brief Calculates and sets t values for each result struct.
 *        t = 2 *i / tCount - 1
 * @param results Results array.
 * @param startIndex offset start index.
 * @param endIndex offset end index.
 * @param tCount Given constant needed to preform the t value calculation.
**/
void calculateTimes(criteria_t *results, int startIndex, int endIndex, int tCount);

/**
 * @brief Prints the results array to stdout.
 * @param results Results array.
 * @param tCount Number of expected results.
**/
void printResults(criteria_t *results, int tCount);

/**
 * @brief Checks which points satisfies the proximity criteria for the allocated times.
 *        This function utlizes CUDA to operate.
 * @param h_points Host's array of points.
 * @param size Size of points array.
 * @param h_result Host's results array.
 * @param chunk Size of results array.
 * @param maximumDistance Maximum distance allowed between the reference point to other points to satisfiy the criteria.
 * @param minimumPoints Minimum points wihin the distance threshold required around the reference point to satisfiy the criteria.
**/
void computeProximities(Point *h_points, int size, criteria_t *h_results, int chunk, double maximumDistance, int minimumPoints);
