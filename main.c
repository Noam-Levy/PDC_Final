#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include <math.h>
#include "util.h"

#define STR_MAX 255

void main(int argc, char* argv[])
{
  int nProc, rank, chunk, i;
  double* timesArr;
  char filename[STR_MAX];
  metadata* data;
  criteria_t** results;
  MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nProc);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == MASTER && nProc != 2)
  {
    fprintf(stderr, "Please run the program with 2 processes only\n");
    MPI_Abort(MPI_COMM_WORLD, __LINE__);
  }

  //define MPI datatypes
  metadata md;
  int md_blockLength[5] = {1, 1, 1, 1, 0};
  MPI_Datatype md_blockTypes[5] = {MPI_INT, MPI_DOUBLE, MPI_INT, MPI_INT, MPI_UB};
  MPI_Datatype MPI_metadata = defineMPIDatatype(&md, md_blockLength, md_blockTypes, 5);

  Point pt;
  int point_blockLength[8] = {1, 1, 1, 1, 1, 1, 1, 0};
  MPI_Datatype point_blockTypes[8] = {MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_INT, MPI_UB};
  MPI_Datatype MPI_point = defineMPIDatatype(&pt, point_blockLength, point_blockTypes, 8);

  if (rank == MASTER)
  {
    argc < 2 ? strcpy(filename, "./input.txt") : strcpy(filename, argv[1]);
    // route stdout to output file
    FILE* output = freopen("output.txt", "w", stdout);
    if (output == NULL)
    {
        fprintf(stderr, "Failed to route stdout to output file. Aborting...\n");
        deallocateMetadata(data);
        return;
    }

    data = readData(filename);
    if (!data)
    {
      fprintf(stderr, "Failed reading data from file. Terminating...\n");
      return;
    }

    MPI_Send(data, 1, MPI_metadata, 1, TAG, MPI_COMM_WORLD);
    for (i = 0; i < data->N; i++)
      MPI_Send(data->points[i], 1, MPI_point, 1, TAG, MPI_COMM_WORLD);
  }
  else // Slave process
  {
    data = (metadata*)malloc(sizeof(metadata));
    MPI_Recv(data, 1, MPI_metadata, MASTER, TAG, MPI_COMM_WORLD, &status);
    data->points = (Point**)malloc(data->N * sizeof(Point*));
    for (i = 0; i < data->N; i++)
    {
      data->points[i] = allocatePoint(data->N);
      MPI_Recv(data->points[i], 1, MPI_point, MASTER, TAG, MPI_COMM_WORLD, &status);
    }
  }

  // calculate t values and share with other process
  chunk = (int)(floor(data->tCount / nProc) + ((data->tCount % nProc) * !rank)); // master process recieves bigger chunk
  timesArr = (double*)malloc((data->tCount + 1) * sizeof(double));
  results = (criteria_t**)malloc((data->tCount + 1) * sizeof(criteria_t*));
  if (!timesArr || !results)
  {
    fprintf(stderr, "Failed allocating memory on process %d\nAborting...", rank);
    MPI_Abort(MPI_COMM_WORLD, __LINE__);
  }
  for (i = 0; i <= data->tCount; i++)
  {
    results[i] = (criteria_t*)malloc(sizeof(criteria_t));
    if (!results[i])
    {
      fprintf(stderr, "Failed allocating memory on process %d\nAborting...", rank);
      MPI_Abort(MPI_COMM_WORLD, __LINE__);
    }
  }
  
  int lower = chunk * rank, 
      upper = data->tCount - chunk + lower,
      offset = rank != 0 ? 0 : upper;
  calculateTimes(timesArr, lower, upper, data->tCount);
  MPI_Sendrecv(timesArr + lower, chunk, MPI_DOUBLE, !rank, TAG, timesArr + offset, data->tCount - chunk, MPI_DOUBLE, !rank, TAG, MPI_COMM_WORLD, &status);

  for (i = lower; i <= upper; i++)
  {
    double t = timesArr[i];
    setPointsPositions(data->points, data->N, t);
    calculateDistances(data->points, data->N);
    checkProximityCriteria(data->points, data->N, data->D, data->K, t, results[i]);
    if (results[i] == NULL) // error while checking proximity criteria. error message will be printed by the check function.
      MPI_Abort(MPI_COMM_WORLD, __LINE__);
  }
  MPI_Barrier(MPI_COMM_WORLD);
  if (rank != MASTER) // slave sends to master its results
  {
    for (i = lower; i <= upper; i++)
    {
      criteria_t* res = results[i];
      MPI_Send(&res->t, 1, MPI_DOUBLE, MASTER, TAG, MPI_COMM_WORLD);
      MPI_Send(&res->isFound, 1, MPI_INT, MASTER, TAG, MPI_COMM_WORLD);
      if (res->isFound)
        MPI_Send(res->pointIDs, 3, MPI_INT, MASTER, TAG, MPI_COMM_WORLD);
      free(res);
    }
  }
  else
  {
    for (i = chunk; i <= data->tCount; i++)
    {
      criteria_t* res = results[i];
      MPI_Recv(&res->t, 1, MPI_DOUBLE, 1, TAG, MPI_COMM_WORLD, &status);
      MPI_Recv(&res->isFound, 1, MPI_INT, 1, TAG, MPI_COMM_WORLD, &status);
      if (res->isFound)
        MPI_Recv(res->pointIDs, 3, MPI_INT, 1, TAG, MPI_COMM_WORLD, &status);
    }
    
    int found = 0;
    # pragma omp parallel for ordered shared(found)
    for (i = 0; i <= data->tCount; i++)
    {
      criteria_t* res = results[i];
      #pragma omp ordered
      if (res->isFound)
      {
        printf("Points ");
        for (int j = 0; j < MIN_CRITERIA_POINTS - 1; j++)
          printf("%d, ", res->pointIDs[j]);
        printf("%d satisfy Proximity Criteria at t=%.2f\n", res->pointIDs[MIN_CRITERIA_POINTS - 1], res->t);
        found = 1;
      }
      free(res);
    }
    if (found == 0)
    {
      printf("There were no %d points found for any t.\n", MIN_CRITERIA_POINTS);
    }
  }

  deallocateMetadata(data);
  free(results);
  MPI_Type_free(&MPI_point);
  MPI_Type_free(&MPI_metadata);
  MPI_Finalize();
}
