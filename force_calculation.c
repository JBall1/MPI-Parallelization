/*
Project 5. Parallelize the force calculation for n particles so that 
           the computation loads on all processes are balanced, and 
           commuication cost has a complexity of O(n log_2 p) for 
           n particles and p processors.
*/
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

//Had to change this from doubles to int's, otherwise won't compile..
#define sgn(x) (((x) < 0) ? (-1) : (1))
#define c1 1.23456
#define c2 6.54321
#define n 123456

//Load balance - partition .5n(n-1) where n%2p == 0

/* Input: n, x[n]. Note that x[i] \not= x[j] for different i, j.
   Output: f[n].
*/
//Changed from calc-force to calc_force as a hyphen is non-standard and won't compile.
//Had to remove n, as it was already defined in the program and this is an expansion of a marco(wont work...)
void calc_force(double *x, double *f, int pid, int np) {
   int  i, j, k, sqrtp, count, pidx, pidy, *hprocs, *vprocs;
   double diff, tmp;

   MPI_Group MPI_GROUP_WORLD, hgroup, vgroup ;
   MPI_Comm hcomm, vcomm ;
   sqrtp = (int) sqrt(np);
   count = n/sqrtp;
   pidx = pid%sqrtp;
   pidy = pid/sqrtp;

   hprocs = (int *) calloc(sqrtp, sizeof(int));
   vprocs = (int *) calloc(sqrtp, sizeof(int));
   //Setup horizontal and vertical procs
   for(i=0; i<sqrtp; i++) {
     hprocs[i] = i + pidy*sqrtp ;
     vprocs[i] = pidx + i*sqrtp ;
}
    MPI_Comm_group(MPI_COMM_WORLD, &MPI_GROUP_WORLD);
    MPI_Group_incl(MPI_GROUP_WORLD, sqrtp, hprocs, &hgroup);
    MPI_Group_incl(MPI_GROUP_WORLD, sqrtp, vprocs, &vgroup);
    MPI_Comm_create(MPI_COMM_WORLD, hgroup, &hcomm);
    MPI_Comm_create(MPI_COMM_WORLD, vgroup, &vcomm);

    MPI_Group_free(&MPI_GROUP_WORLD);
    MPI_Group_free(&hgroup);
    MPI_Group_free(&vgroup);
    free(hprocs); 
    free(vprocs);


    int  i_local, h_root, v_root ;
    int  *hbuf, *vbuf ;

    hbuf = (int *)calloc(count, sizeof(int));
    vbuf = (int *)calloc(count, sizeof(int));
   //init array of 0's, no change
   for(i=0; i<n; i++) f[i] = 0.0;
   //Define the amount of elements remaining for the code to go through. math per lecture.
   double rem_elem = n/count;
   for(i=1; i<n; i++){
      for(j=0; j<i; j++) {

       h_root= (i-1)/count;
       v_root = h_root;
       i_local = (i-1)%count;
        //Area where parallelization is needed
       if(pidx==h_root) {
                diff = x[i] - x[j];
                tmp = c1/(diff*diff*diff) - c2*sgn(x)/(diff*diff) ;
                hbuf[i] += tmp;
                hbuf[j] -= tmp;

       }
       MPI_Bcast(hbuf, count, MPI_INT, h_root, hcomm) ;

      if(pidy==v_root) {
                diff = x[i] - x[j];
                tmp = c1/(diff*diff*diff) - c2*sgn(x)/(diff*diff) ;
               // f[i] += tmp;
               // f[j] -= tmp;
                //Need to change this, may be the issue
                vbuf[i] += tmp;
                vbuf[j] -= tmp;
          
       }
       MPI_Bcast(vbuf, count, MPI_INT, v_root, vcomm) ;

       for(i=0; i<n; i++){
           for(j = 0; j<i; j++){
            f[i] = (vbuf[i] + hbuf[j])
            }
           }
    }
   }

    free(hbuf); free(vbuf);
}


int main(int argc, char* argv[]){
    int pid, np, mtag, count;
    MPI_Status status;
    MPI_Request req_s;
    MPI_Init( &argc ,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    //define f, x of size n.
    int f[n];
    int x[n];
    calc_force(*x,*f,pid,np);

    MPI_Finalize();

return 0;
}
