/*
computation of pi using pthreads

	Pi is approximated by using the adaptive 
	quadrature routine. It works by calculating
	the area of the upper left quadrant of the 
	unit circle, and multiplying the result by 4.
	It is approximated within the error estimator 
	epsilon. Smaller epsilon = higher number of 
	recursions = more accurate estimation.
	
	Features:		Each thread gets a partition of the
					area to calculate. The threads' results
					are passed in a global array, and the main
					method sums them up.				
		
	usage under Linux:
		gcc pi.c -lpthread
		./a.out np epsilon
*/

#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<pthread.h>
#include<sys/time.h>
#include<stdbool.h>

#define MAXWORKERS 10		/* max number of workers */
#define MINEPSILON 1e-20	/* the smallest value of epsilon */
double epsilon;				/* error estimator */
int np;						/* number of threads */
double result[MAXWORKERS];  /* array with partial results */

double start_time, end_time; /* start and end times */

 /* timer */
double read_timer() {
  static bool initialized = false;
  static struct timeval start;
  struct timeval end;
  if( !initialized )
  {
    gettimeofday( &start, NULL );
    initialized = true;
  }
  gettimeofday( &end, NULL );
  return (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
}

/* y = f(x) = the height of the unit circle given an x-coordinate */
double f(double x){

	return sqrt(1 - pow(x, 2));
}

/* serial version of the adaptive quadrature routine */
double quad(double l, double r, double fl, double fr, double area) {

    /* calculating the new parameters using the 
       adaptive quadrature routine formulas */
	double m = (l + r)/2;
	double fm = f(m);
	double larea = (fl + fm)*(m - l)/2;
	double rarea = (fm + fr)*(r - m)/2;
	/* if the result is not within the given error estimate,
	   continue the recursion  */
	if (fabs((larea + rarea) - area) > epsilon) {
		larea = quad(l, m, fl, fm, larea);
		rarea = quad(m, r, fm, fr, rarea);
	}
	
	return (larea + rarea);
}

/* the arguments each thread passes to parallel_quad */
struct args{double l, r, fl, fr, area; long id;};

/* parallel version of the adaptive quadrature routine */
void *parallel_quad(void *myargs) {

	struct args *args = myargs;

	/* passing the arguments to local variables */
    double l = args->l;
    double r = args->r;
    double fl = args->fl;
    double fr = args->fr;
    double area = args->area;

    /* calculating the new parameters using the 
       adaptive quadrature routine formulas */
    double m = (l+r)/2;
    double fm = f(m);
    double larea = (fl+fm)*(m-l)/2;
    double rarea = (fm+fr)*(r-m)/2;
	/* if the result is not within the given error estimate,
	   continue the recursion  */	
	if (fabs((larea + rarea) - area) > epsilon) {
		args->l = l;
		args->r = m;
		args->fl = fl;
		args->fr = fm;
		args->area = larea;

		parallel_quad(args);
	
		args->l = m;
		args->r = r;
		args->fl = fm;
		args->fr = fr;
		args->area = rarea;
			
		parallel_quad(args);

	} else {
		result[args->id] += larea + rarea;
	}
}

int main(int argc, char *argv[]){
	
	/* read command line arguments */
	np = (argc > 1)? atoi(argv[1]) : MAXWORKERS;
	epsilon = (argc > 2)? atof(argv[2]) : MINEPSILON;
	np = (np > MAXWORKERS)? MAXWORKERS : np;
	epsilon = (epsilon < MINEPSILON)? MINEPSILON : epsilon;
	
	/* serial quad */
	if(np == 0){
		start_time = read_timer();
		double area = quad(0, 1, f(0), f(1), (f(0) + f(1))*(1 - 0)/2);
		end_time = read_timer();
		double pi = 4 * area;
		double timeElapsed = end_time - start_time;
		/* print the results */
		printf("epsilon = %g\n", epsilon);	
		printf("pi = %.20f\n", pi);
		printf("execution time = %g s\n", timeElapsed);
		
	/* parallel quad */
	} else {	
		long i; /* use long in case of a 64-bit system */
		pthread_t workerid[MAXWORKERS];
	
		/* initializing parameters */
		struct args args[np];

		double area;
		/* the width of the area each thread calculates */
		double dxPerThread = 1 / ((double)np);

		/* get start time */
		start_time = read_timer();

		/* do the parallel work: create the workers */
		for (i = 0; i < np; i++){
			args[i].id = i;
			args[i].l = i * dxPerThread;
			args[i].r = args[i].l + dxPerThread;
			args[i].fl = f(args[i].l);
			args[i].fr = f(args[i].r);
			args[i].area = (f(args[i].l)+f(args[i].r))*(args[i].r-args[i].l)/2;
			pthread_create(&workerid[i], NULL, parallel_quad, &args[i]);       
		}
		
		/* wait for all threads to terminate */
		for (i = 0; i < np; i++)
	   		pthread_join(workerid[i], NULL);
	 	
	 	/* get end time */
	 	end_time = read_timer();
	 
	 	/* summing up the area calculated by the workers */	
	 	for(i = 0; i < np; i++)
	 		area += result[i];
	
		double pi = 4 * area;
		double timeElapsed = end_time - start_time;
		/* print the results */
		printf("epsilon = %g\n", epsilon);	
		printf("pi = %.20f\n", pi);
		printf("real pi = %.20f\n", 3.14159265358979323846);
		printf("execution time = %g s\n", timeElapsed);
	}
	
	return 0;
}

