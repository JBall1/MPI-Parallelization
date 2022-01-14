/*
By Joshua Ball

*/

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define generate_data(i,j)  (i)+(j)*(j)

int main( int argc, char **argv)
{
  int i, j, pid, np, mtag, count;
  double t0, t1 ;
  int data[100][100], row_sum[100], partialSumOne[25], partialSumTwo[25] ;
  MPI_Status status;
  MPI_Request req_s, req_r, req_i;


  MPI_Init( &argc, &argv );
  MPI_Comm_rank(MPI_COMM_WORLD, &pid);
  MPI_Comm_size(MPI_COMM_WORLD, &np);

  if(pid == 0) { // generate data[]
    for(i=0; i<50; i++)
      for(j=0; j<100; j++)
        data[i][j] = generate_data(i,j) ;
    mtag = 1 ;
    MPI_Isend(data, 5000, MPI_INT, 1, mtag, MPI_COMM_WORLD, &req_s) ;
    for(i=50; i<100; i++)
      for(j=0; j<100; j++)
        data[i][j] = generate_data(i,j) ;

    for(i=50; i<100; i++) {
      row_sum[i] = 0 ;
      for(j=0; j<100; j++)
         row_sum[i] += data[i][j] ;
    }
    MPI_Wait(&req_s, &status) ;

   /*** receive computed row_sums from  pid 1 ***/
    mtag = 2 ;
    MPI_Recv(row_sum, 50, MPI_INT, 1, mtag, MPI_COMM_WORLD, &status) ;
    for(i=0; i<100; i++) {
       printf(" %d  ", row_sum[i]) ;
       if(i%5 ==0) printf("\n");
    }
  }
  
  else { /*** pid == 1 ***/
  /*
  This section of the code receives a total of 5000 elements of [int][int] data. Its task is to ensure that it has received it all,
  then to compute the row sums of the 2d array. It needs to do the computation and communication(sending of row sums) asynchronously.
  
  To do this, I have utlized non-blocking statements, such as Irecv and Isend as well as divided the row sum computation work in half by using two different loops.
  We then send this data(non-blocking) as the loops complete and wait for our flag, req_s in this case, to show that all 50 have been sent.
  */

    mtag = 1 ;
    MPI_Irecv(data, 5000, MPI_INT, 0, mtag, MPI_COMM_WORLD, &req_r);
    MPI_Wait(&req_r, &status); // must wait until all of data[][] is recv'd in order to process it.
    
    //splitting the sum into two different parts, here we calculate the row sum
    //for [0:24]
    for(i = 0; i < 25; i++){
      row_sum[i] = 0;
      for(j = 0; j < 100; j++){
        row_sum[i] += data[i][j];
      }
    }
    //My initial idea was to place the Isend here, between the for loops but doing this results in 0's for the elements in row_sum[25:49].
    //here we calculate the row sum for [25:49]
    for(i = 25; i < 50; i++){
      row_sum[i] = 0;
      for(j = 0; j < 100; j++){
        row_sum[i] += data[i][j];
      }
    }
    //Here we change the tag to the same one as the recv'r
    //We set the send count to 50, the same size as the recv'rs buffer.
    mtag = 2;
    //Again, we utilize Isend to allow for the non-blocking transmission of the row sums calcualted in the loops above.
    MPI_Isend(row_sum, 50, MPI_INT, 0, mtag, MPI_COMM_WORLD, &req_s);
    // we then wait for the sending process to complete for all 50 items
    MPI_Wait(&req_s, &status);
  }

  MPI_Finalize();

  return 1;
} /****************** End of function main() ********************/




   
