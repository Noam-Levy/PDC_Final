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
}
