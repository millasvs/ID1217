/*

To run in Linux:
make -f Makefile_p pthreads
./pthreads

*/


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <pthread.h>
#include "common.h"

//
//  global variables
//
int n, n_threads;
particle_t *particles;
bin_t *bins;
FILE *fsave;
pthread_barrier_t barrier;
extern int num_bins, num_rows; 

int *globalIds; 

//
//  check that pthreads routine call was successful
//
#define P( condition ) {if( (condition) != 0 ) { printf( "\n FAILURE in %s, line %d\n", __FILE__, __LINE__ );exit( 1 );}}

//
//  This is where the action happens
//
void *thread_routine( void *pthread_id )
{
    int thread_id = *(int*)pthread_id;

    int particles_per_thread = (n + n_threads - 1) / n_threads;
    int first = min(  thread_id    * particles_per_thread, n );
    int last  = min( (thread_id+1) * particles_per_thread, n );
    //
    //  simulate a number of time steps
    //
    for( int step = 0; step < NSTEPS; step++ )
    {
        //
        //  compute forces
        //
        for( int i = first; i < last; i++ )
            particles[i].ax = particles[i].ay = 0;

        for (int i = 0; i < num_bins; i++)
			go_through_neighbors(particles, bins, first, last, i);

        pthread_barrier_wait( &barrier );

        //
        //  move particles
        //particles_per_thread
		for (int i = first; i < last; i++) 
            move_and_update( particles[i], i, globalIds[i] );		
				
        pthread_barrier_wait( &barrier );        

		if(thread_id==0){
			insert_into_bins(particles, bins, n);
        }
                
        pthread_barrier_wait( &barrier );        
        
#ifdef DEBUG
		/* checking that the number of particles doesnt change */
        int numparticles = 0;
        for(int i = 0; i < num_bins; i++)
        	numparticles += bins[i].num_particles;
        assert(numparticles==n);
#endif 
        
        //
        //  save if necessary
        //
        if( thread_id == 0 && fsave && (step%SAVEFREQ) == 0 )
            save( fsave, n, particles );
    }
    
    return NULL;
}

//
//  benchmarking program
//
int main( int argc, char **argv )
{    
    //
    //  process command line
    //
    if( find_option( argc, argv, "-h" ) >= 0 )
    {
        printf( "Options:\n" );
        printf( "-h to see this help\n" );
        printf( "-n <int> to set the number of particles\n" );
        printf( "-p <int> to set the number of threads\n" );
        printf( "-o <filename> to specify the output file name\n" );
        return 0;
    }
    
    n = read_int( argc, argv, "-n", 1000 );
    n_threads = read_int( argc, argv, "-p", 2 );
    char *savename = read_string( argc, argv, "-o", NULL );
    
    //
    //  allocate resources
    //
    fsave = savename ? fopen( savename, "w" ) : NULL;

    particles = (particle_t*) malloc( n * sizeof(particle_t) );
    set_size( n );
	globalIds =  (int*) malloc(n * sizeof(int));
    init_particles( n, particles );

	bins = (bin_t*) malloc( num_bins * sizeof(bin_t) );
	
	/* make room for up to n particles per bin */
	for(int i = 0; i < num_bins; i++)
		bins[i].particle_ids = (int*) malloc( n * sizeof(int));


	/* initialise the bins */
    init_bins(bins);

	for(int i = 0; i < n; i++)
		globalIds[i] = (int)(floor(particles[i].x / cutoff) * num_rows + floor(particles[i].y / cutoff));

	/* insert particles into the bins */
  	insert_into_bins(particles, bins, 0, n, n);

    pthread_attr_t attr;
    P( pthread_attr_init( &attr ) );
    P( pthread_barrier_init( &barrier, NULL, n_threads ) );

    int *thread_ids = (int *) malloc( n_threads * sizeof( int ) );
    for( int i = 0; i < n_threads; i++ ) 
        thread_ids[i] = i;

    pthread_t *threads = (pthread_t *) malloc( n_threads * sizeof( pthread_t ) );
    
    //
    //  do the parallel work
    //
    double simulation_time = read_timer( );
    for( int i = 1; i < n_threads; i++ ) 
        P( pthread_create( &threads[i], &attr, thread_routine, &thread_ids[i] ) );
    
    thread_routine( &thread_ids[0] );
    
    for( int i = 1; i < n_threads; i++ ) 
        P( pthread_join( threads[i], NULL ) );
    simulation_time = read_timer( ) - simulation_time;
    
    printf( "n = %d, n_threads = %d, simulation time = %g seconds\n", n, n_threads, simulation_time );
    
    //
    //  release resources
    //
    P( pthread_barrier_destroy( &barrier ) );
    P( pthread_attr_destroy( &attr ) );
    free( thread_ids );
    free( threads );
    free( particles );
    free( globalIds );
    free( bins );
    if( fsave )
        fclose( fsave );
    
    return 0;
}
