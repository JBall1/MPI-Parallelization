#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

//Macros to define Min and Len functions
//LEN: given some array, returns the length of the row, col
//MIN: given two values, returns the min value
#define LEN(arr) ((int) (sizeof (arr) / sizeof (arr)[0]))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
//Define the n value for matrix of size n x n
#define n 8
//global defintions require in C to define the size of our matrix and other values during preprocessing
#define i_start 0
#define j_start 0
//#define i_end i_start
//#define j_end j_start
//Define i_start, i_end = sizezof data + i_start or n/sqrt(p)
//Define j_start, j_end

int main(int argc, char* argv[]){

    //MPI Init variables
    int pid, np, mtag, count;
    MPI_Status status;
    MPI_Request req_s, req_r, req_i;
    int ranks[] = {0};
    MPI_Init(&argc, &argv );
    MPI_Group gpone, gptwo;
    MPI_Comm newcomm;
    MPI_Comm_group(MPI_COMM_WORLD, &gpone);
    //MPI init function calls
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &np);
    MPI_Group_excl(gpone,1,ranks,&gptwo);
    MPI_Comm_create(MPI_COMM_WORLD,gptwo,&newcomm);
    
    int i_end = np/sqrt(n) + i_start;
    int j_end = np/sqrt(n) + j_start;

    //Defined an arb. matrix of 7x7 for testing purposes only. This will function as our input matrix,
    //which is n x n size, with 0's on the diagonals
    int D0[n][n] = {{0 , 2, 4, 1, 5, 32, 0}, 
                        {1 , 0, 4, 1, 5,  0, 3},
                        {1 , 9, 0, 1, 0,  2, 2},
                        {2 , 2, 4, 0, 5,  3, 5},
                        {1 , 2, 0, 1, 0,  7, 7},
                        {1 , 0, 4, 1, 5,  0, 6},
                        {0 , 2, 4, 1, 5,  9, 0}};

    int i, j;
    int D[n][n]; // only needs to be created once to store all values from all processes, allowing memory to be saved.
    //Nested loops to find min in D0
    for(int k = 0; k < n-1; k++){
      for(i = i_start; i < i_end - 1; i++){
        for(j = j_start; j < j_end - 1; j++){ //Partition//para me
          D0[i][j] = MIN(D0[i][j], D0[i][k] + D0[k][j]);
        }
      }
      for(i = i_start; i < i_end - 1; i++){
        for(j = j_start; j < j_end - 1; j++){
          D0[i][j] = D[i][j];
        }
      }
    }//end of main for loop k   
    MPI_Group_free(&gpone);
    MPI_Group_free( &gptwo);
    MPI_Finalize();

    //Does not return D as this was not written as a seperate fucntion, but this code could be placed into a function, returning the matrix D if required 
    //farily easily.
    return 0;
}