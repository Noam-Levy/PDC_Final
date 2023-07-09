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
  for (int i = 0; i <= tCount; i++)
  {
    float t = 2.0 * i / tCount - 1;
    setPointsPositions(data->points, size, t);
    calculateDistances(data->points, size);
    checkProximityCriteria(data->points, size, data->D, data->K, t);
  }

  //TODO: deallocate everything
}
