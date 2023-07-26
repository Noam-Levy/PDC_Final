#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "util.h"



void main(int argc, char* argv[])
{
  char filename[STR_MAX];
  int N, K, tCount, i;
  double D, t;
  Point* points;

  argc < 2 ? strcpy(filename, "./input.txt") : strcpy(filename, argv[1]);
  
  points = readData(filename, &N, &K, &D, &tCount);
  if (!points)
  {
    fprintf(stderr, "Terminating...\n");
    return;
  }

  int totalSatisfiedCounter = 0;
  int temp;
  FILE *output = freopen("output.txt", "w", stdout);
  if (output == NULL) // route stdout to output file
  {
   fprintf(stderr, "Failed to route stdout to output file. Aborting...\n");
   return;
  }

  for (i = 0; i <= tCount; i++)
  {
    t = 2.0 * i / tCount - 1;
    setPointsPositions(points, N, t);
    calculateDistances(points, N);
    temp = checkProximityCriteria(points, N, D, K, t);
    if (temp < 0) // error while checking proximity criteria. error will be printed by the check function
      break;
    totalSatisfiedCounter += temp;
  }
  
  // if no error occourred and need to print not found message
  if (temp >= 0 && totalSatisfiedCounter == 0)
      printf("There were no %d points found for any t.\n", MIN_CRITERIA_POINTS);

  fclose(output); // close output file
  for (i = 0; i < N; i++)
    free(points[i].distances);
  free(points);
}
