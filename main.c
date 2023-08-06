#include <string.h>
#include <mpi.h>
#include "util.h"

int main(int argc, char* argv[])
{
  FILE* output;
  char filename[STR_MAX];
  int N, K, tCount, nProc, rank, chunk, remainder, startIndex, endIndex, i;
  double D, t, startTime, endTime;
  Point* points;
  criteria_t* localResults, *globalResults;
  MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nProc);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == MASTER && nProc < 2)
  {
    fprintf(stderr, "Please run the program with at least 2 processes\n");
    MPI_Abort(MPI_COMM_WORLD, __LINE__);
  }

  // define MPI_POINT datatype
  MPI_Datatype MPI_POINT;
  MPI_Type_contiguous(sizeof(Point), MPI_BYTE, &MPI_POINT);
  MPI_Type_commit(&MPI_POINT);

  if (rank == MASTER)
  {
    argc < 2 ? strcpy(filename, "./input.txt") : strcpy(filename, argv[1]);
    startTime = MPI_Wtime();
    points = readData(filename, &N, &K, &D, &tCount);
    if (!points)
    {
      fprintf(stderr, "Terminating...\n");
      MPI_Abort(MPI_COMM_WORLD, __LINE__);
    }
    globalResults = (criteria_t*)malloc((tCount + 1) * sizeof(criteria_t));
    if (!globalResults)
    {
      fprintf(stderr, "Faild allocating global results array. Terminating...\n");
      MPI_Abort(MPI_COMM_WORLD, __LINE__);      
    }
    // route stdout to output file
    output = freopen("output.txt", "w", stdout);
    if (output == NULL) // route stdout to output file
    {
      fprintf(stderr, "Failed to route stdout to output file. Aborting...\n");
      MPI_Abort(MPI_COMM_WORLD, __LINE__);
    }
  }

  // broadcast metadata
  MPI_Bcast(&N, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
  MPI_Bcast(&K, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
  MPI_Bcast(&D, 1, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);
  MPI_Bcast(&tCount, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
  
  // allocate points array in slave processes
  if (rank != MASTER)
  {
    points = (Point*)malloc(N * sizeof(Point));
    if (!points)
    {
      fprintf(stderr, "Failed to allocate points array in process %d. Aborting...\n", rank);
      MPI_Abort(MPI_COMM_WORLD, __LINE__);
    }
  }
  // broadcast points
  MPI_Bcast(points, N, MPI_POINT, MASTER, MPI_COMM_WORLD);

  chunk = (tCount + 1) / nProc;
  remainder = (tCount + 1) % nProc;
  startIndex = chunk * rank;
  endIndex = startIndex + chunk + remainder;
  // if applicable, allocate remainder tasks to master and offset slaves accordingly
  if (rank == MASTER)
    chunk += remainder;  
  else
    startIndex += remainder;
   
  localResults = (criteria_t*)malloc(chunk * sizeof(criteria_t));
  if (!localResults)
  {
      fprintf(stderr, "Failed allocating memory in process %d. Aborting...\n", rank);
      MPI_Abort(MPI_COMM_WORLD, __LINE__);    
  }
  
  calculateTimes(localResults, startIndex, endIndex, tCount); // calculate process' allocated times
  computeProximities(points, N, localResults, chunk, D, K);  
  MPI_Barrier(MPI_COMM_WORLD);

  if (rank != MASTER) // slaves send their results to master 
  {
    for (i = 0; i < chunk; i++)
    {
      criteria_t res = localResults[i];
      MPI_Send(&res.t, 1, MPI_DOUBLE, MASTER, TAG, MPI_COMM_WORLD);
      MPI_Send(&res.isFound, 1, MPI_INT, MASTER, TAG, MPI_COMM_WORLD);
      if (res.isFound)
        MPI_Send(&res.pointIDs, MIN_CRITERIA_POINTS, MPI_INT, MASTER, TAG, MPI_COMM_WORLD);
    }
  }
  else
  {
    memcpy(globalResults, localResults, chunk * sizeof(criteria_t));
    int found = 0;
    for (i = 0; i <= tCount; i++)
    {
      criteria_t res = globalResults[i];
      if (i >= chunk)
      {
        MPI_Recv(&res.t, 1, MPI_DOUBLE, MPI_ANY_SOURCE, TAG, MPI_COMM_WORLD, &status);
        MPI_Recv(&res.isFound, 1, MPI_INT, MPI_ANY_SOURCE, TAG, MPI_COMM_WORLD, &status);
        if (res.isFound)
          MPI_Recv(&res.pointIDs, MIN_CRITERIA_POINTS, MPI_INT, MPI_ANY_SOURCE, TAG, MPI_COMM_WORLD, &status);
      }
      if (res.isFound == 1)
      {
        printf("Points ");
        for (int j = 0; j < MIN_CRITERIA_POINTS - 1; j++)
          printf("%d, ", res.pointIDs[j]);
        printf("%d satisfy Proximity Criteria at t=%.2f\n", res.pointIDs[MIN_CRITERIA_POINTS - 1], res.t);
        found = 1;
      }
    }

    if (found == 0)
      printf("There were no %d points found for any t.\n", MIN_CRITERIA_POINTS);
    
    // print execution time to console.
    endTime = MPI_Wtime();
    fprintf(stderr, "Execution time: %.4f seconds\n", endTime - startTime);
    free(globalResults);
  }
 
  MPI_Type_free(&MPI_POINT);
  free(points);
  free(localResults);
  MPI_Finalize();
}
