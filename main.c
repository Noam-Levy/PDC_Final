#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include <math.h>
#include "util.h"

#define STR_MAX 255

void main(int argc, char* argv[])
{
  int nProc, rank, chunk;
  double* timesArr;
  char filename[STR_MAX];
  metadata* data;
  MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nProc);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == MASTER && nProc != 2)
  {
    fprintf(stderr, "Please run the program with 2 processes only\n");
    MPI_Abort(MPI_COMM_WORLD, 1);
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
    for (int i = 0; i < data->N; i++)
      MPI_Send(data->points[i], 1, MPI_point, 1, TAG, MPI_COMM_WORLD);
  }
  else // Slave process
  {
    data = (metadata*)malloc(sizeof(metadata));
    MPI_Recv(data, 1, MPI_metadata, MASTER, TAG, MPI_COMM_WORLD, &status);
    data->points = (Point**)malloc(data->N * sizeof(Point*));
    for (int i = 0; i < data->N; i++)
    {
      data->points[i] = allocatePoint(data->N);
      MPI_Recv(data->points[i], 1, MPI_point, MASTER, TAG, MPI_COMM_WORLD, &status);
    }
  }

  // calculate t values and share with other process
  chunk = (int)(floor(data->tCount / nProc) + (data->tCount % nProc) * rank);
  timesArr = (double*)malloc(data->tCount * sizeof(double));
  if (!timesArr)
  {
    fprintf(stderr, "Failed allocating memory on process %d\nAborting...", rank);
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  int lower = chunk * rank, 
      upper = data->tCount - chunk + (chunk * rank),
      offset = rank != 0 ? 0 : upper;
  calculateTimes(timesArr, lower, upper, data->tCount);
  MPI_Sendrecv(timesArr + lower, chunk, MPI_DOUBLE, !rank, TAG, timesArr + offset, data->tCount - chunk, MPI_DOUBLE, !rank, TAG, MPI_COMM_WORLD, &status);

  // copy data to device

  // TODO: compute on GPU

  // TODO: copy results from device

  // TODO: slave sends results to master

  // TODO: master prints results


  MPI_Type_free(&MPI_point);
  MPI_Type_free(&MPI_metadata);
  deallocateMetadata(data);
  free(timesArr);
      // MPI_Finalize(); // TODO: check why throwing error
}


