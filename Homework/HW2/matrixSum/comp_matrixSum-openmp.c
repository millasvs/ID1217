/* matrix summation using OpenMP

   usage with gcc (version 4.2 or higher required):
     gcc -O -fopenmp -o matrixSum-openmp matrixSum-openmp.c 
     ./matrixSum-openmp size numWorkers

*/

#include <omp.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define MAXSIZE 20000		/* maximum matrix size */
#define MAXVALUE 1000		/* upper threshold for value of matrix elements */
#define MAXWORKERS 10   	/* maximum number of workers */

double start_time, end_time; 	/* start and end times */
int numWorkers;           		/* number of workers */ 
int size; 						/* number of rows / number of columns
								   of matrix*/
int matrix[MAXSIZE][MAXSIZE];	/* matrix to be calculated on */

/* check correctness of parallel implementation
   by comparing with result of serial computation */
void check_results(long total, int min, int max){
unsigned long sum = 0;
int amin = INT_MAX;
int amax = 0;
for(int i = 0; i < size; i++){
	for(int j = 0; j < size; j++){
		sum += matrix[i][j];
		if(amin > matrix[i][j]) amin = matrix[i][j];
		if(amax < matrix[i][j]) amax = matrix[i][j];		
	}
}

assert(total == sum);
assert(min == amin);
assert(max == amax);

}

void serial_matrixsum(){

unsigned long sum = 0;
int amin = INT_MAX, amini, aminj;
int amax = 0, amaxi, amaxj;
 for(int i = 0; i < size; i++){
	for(int j = 0; j < size; j++){
		sum += matrix[i][j];
		if(amin > matrix[i][j]){ 
		amin = matrix[i][j];
		amini = i;
		aminj = j;
		}
		if(amax < matrix[i][j]){
		amax = matrix[i][j];
		amaxi = i;
		amaxj = j;
		}		
	}
 }

}

void initialize_matrix(){

  /* initialize the matrix */
  int i, j;
  for (i = 0; i < size; i++) {
	for (j = 0; j < size; j++) {
	  matrix[i][j] = rand()%MAXVALUE;
	  }
  }
}


/* read command line, initialize, and create threads */
int main(int argc, char *argv[]) {

  int i, j;
  
 /* start time, end time and time elapsed
 	for serial matrixsum */
 double s_start_time, s_end_time, s_timeElapsed;
 /* execution time for parallel matrixsum, and
 	speedup for parallel matrixsum */
 double timeElapsed, speedup;

 printf("#	speedup parallel matrixsum\n");
 printf("#processors\tsize\tspeedup\n");
  for(numWorkers = 2; numWorkers < 5; numWorkers++){
	 for(size = 100; size <= MAXSIZE; size+=(size/4)){
		 /* fill the matrix for serial matrixsum */
		initialize_matrix();
	 	s_start_time = omp_get_wtime();
	 	/* perform serial matrixsum */
	 	serial_matrixsum();
	 	s_end_time = omp_get_wtime();
	 	/* calculate execution time of serial matrixsum */
	 	s_timeElapsed = s_end_time - s_start_time;

	 	/* refill the matrix for parallel matrixsum */
	 	initialize_matrix();
	 	unsigned long total = 0;
		int min = INT_MAX, max = 0, min_i, min_j, max_i, max_j;
	 	start_time = omp_get_wtime();
	 	/* perform parallel matrixSum */
		#pragma omp parallel num_threads(numWorkers)
		{
		int my_min = INT_MAX, my_max = 0, mmin_i, mmin_j, mmax_i, mmax_j; 
		#pragma omp for reduction (+:total) private(j)
		  for (i = 0; i < size; i++){
			for (j = 0; j < size; j++){
			  total += matrix[i][j];
			  if(matrix[i][j] > my_max){ 
			  	my_max = matrix[i][j];
			  	mmax_i = i;
			  	mmax_j = j;
			  }
			  if(matrix[i][j] < my_min){
			  	my_min = matrix[i][j];
			  	mmin_i = i;
			  	mmin_j = j;
			  }
			}
		   }
		// implicit barrier

		if(my_min < min){
		#pragma omp critical
		{
			min = my_min;
			min_i = mmin_i;
			min_j = mmin_j;
		}
		}
	
		if(my_max > max){
		#pragma omp critical
		{
			max = my_max;
			max_i = mmax_i;
			max_j = mmax_j;
		}
		}
		}
		
	 	end_time = omp_get_wtime();
	 	/* check that the results are correct */	 	
		check_results(total, min, max);
	 	/* calculate execution time of parallel matrixsum */	 	
	 	timeElapsed = end_time - start_time;
	 	/* calculate speedup by dividing serial execution
	 	   time with parallel execution time */
	 	speedup = s_timeElapsed / timeElapsed;
	 	/* print the results */
	 	printf("%d\t\t%d\t%g\n", numWorkers, size, speedup);
	 }
 }

}

