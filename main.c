#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "util.h"

#define STR_MAX 255

void main(int argc, char* argv[])
{
  char filename[STR_MAX];
  metadata* data;

  argc < 2 ? strcpy(filename, "./input.txt") : strcpy(filename, argv[1]);
  
  data = readData(filename);
  if (!data)
  {
    fprintf(stderr, "Failed reading data from file. Terminating...\n");
    return;
  }
  int tCount = data->tCount;
  int size = data->N;
  int totalSatisfiedCounter = 0;
  int temp;
  if(freopen("output.txt", "w", stdout) == NULL) // route stdout to output file
  {
   fprintf(stderr, "Failed to route stdout to output file. Aborting...\n");
   deallocateMetadata(data);
   return;
  }

  for (int i = 0; i <= tCount; i++)
  {
    float t = 2.0 * i / tCount - 1;
    setPointsPositions(data->points, size, t);
    calculateDistances(data->points, size);
    temp = checkProximityCriteria(data->points, size, data->D, data->K, t);
    if (temp < 0) // error while checking proximity criteria. error will be printed by the check function
      break;
    totalSatisfiedCounter += temp;
  }
  
  // if no error occourred and need to print not found message
  if (temp >= 0 && totalSatisfiedCounter == 0)
      printf("There were no %d points found for any t.\n", MIN_CRITERIA_POINTS);
  

  fclose(stdout); // close output file
  deallocateMetadata(data);
}
