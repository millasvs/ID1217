/*
Run with #threads number of threads:
make mpi
mpirun -n #threads ./mpi

*/


#include <mpi.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <stddef.h>
#include "common.h"
extern int num_bins, num_rows; 
int *globalIds;


//
//  benchmarking program
//
int main( int argc, char **argv )
{    
    //
    //  process command line parameters
    //
    if( find_option( argc, argv, "-h" ) >= 0 )
    {
        printf( "Options:\n" );
        printf( "-h to see this help\n" );
        printf( "-n <int> to set the number of particles\n" );
        printf( "-o <filename> to specify the output file name\n" );
        return 0;
    }
    
    int n = read_int( argc, argv, "-n", 1000 );
    char *savename = read_string( argc, argv, "-o", NULL );
    
    //
    //  set up MPI
    //
    int n_proc, rank;
    MPI_Init( &argc, &argv );
    MPI_Comm_size( MPI_COMM_WORLD, &n_proc );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank ); 

    //
    //  allocate generic resources
    //
    FILE *fsave = savename && rank == 0 ? fopen( savename, "w" ) : NULL;
    particle_t *particles = (particle_t*) malloc( n * sizeof(particle_t) );
	
    MPI_Datatype PARTICLE;
    MPI_Type_contiguous( 6, MPI_DOUBLE, &PARTICLE );
    MPI_Type_commit( &PARTICLE );
	
	set_size( n );
	
    //
    //  set up the data partitioning across processors
    //
    int particle_per_proc = (n + n_proc - 1) / n_proc;
    int *partition_offsets = (int*) malloc( (n_proc+1) * sizeof(int) );

    for( int i = 0; i < n_proc+1; i++ )
        partition_offsets[i] = min( i * particle_per_proc, n );
    
    int *partition_sizes = (int*) malloc( n_proc * sizeof(int) );
    for( int i = 0; i < n_proc; i++ )
        partition_sizes[i] = partition_offsets[i+1] - partition_offsets[i];
        
    //
    //  allocate storage for local partition
    //
    int nlocal = partition_sizes[rank];
    particle_t *local = (particle_t*) malloc( nlocal * sizeof(particle_t) );
    
    globalIds = (int*) malloc(n * sizeof(int));

    bin_t *bins = (bin_t*) malloc( num_bins * sizeof(bin_t) );
	
	int **bins_particle_ids;
	bins_particle_ids = (int**) malloc(num_bins * n * sizeof(int));
	for(int i = 0; i < num_bins; i++)
		bins_particle_ids[i] = (int*) malloc(n * sizeof(int));
		
	MPI_Datatype BIN;
    MPI_Type_contiguous( 1, MPI_INT, &BIN );
    MPI_Type_commit( &BIN );
   
    //
    //  initialize and distribute the particles (that's fine to leave it unoptimized)
    //
    
    if( rank == 0 ){
        init_particles( n, particles );
        
        for(int i = 0; i < n; i++)
	        globalIds[i] = (int)(floor(particles[i].x / cutoff) * num_rows + floor(particles[i].y / cutoff));
        
        insert_into_bins(particles, bins, bins_particle_ids, n);
        
	}
	
    MPI_Scatterv( particles, partition_sizes, partition_offsets, PARTICLE, local, nlocal, PARTICLE, 0, MPI_COMM_WORLD );
    //MPI_Scatterv( globalIds, partition_sizes, partition_offsets, MPI_INT, local_global_ids, nlocal, MPI_INT, 0, MPI_COMM_WORLD );
    
    MPI_Bcast(bins, num_bins, BIN, 0, MPI_COMM_WORLD);

	MPI_Bcast(bins_particle_ids, num_bins, MPI_INT, 0, MPI_COMM_WORLD);
	for (int i = 0; i < num_bins; i++)
	    MPI_Bcast(bins_particle_ids[i], n, MPI_INT, 0, MPI_COMM_WORLD);
	       

    //
    //  simulate a number of time steps
    //
    double simulation_time = read_timer( );
    for( int step = 0; step < NSTEPS; step++ )
    {
        // 
        //  collect all global data locally (not good idea to do)
        //	
        
        MPI_Allgatherv( local, nlocal, PARTICLE, particles, partition_sizes, partition_offsets, PARTICLE, MPI_COMM_WORLD );
        //MPI_Allgatherv( local_global_ids, nlocal, MPI_INT, globalIds, partition_sizes, partition_offsets, MPI_INT, MPI_COMM_WORLD );

	    MPI_Bcast(bins, num_bins, BIN, 0, MPI_COMM_WORLD);

    	MPI_Bcast(bins_particle_ids, num_bins, MPI_INT, 0, MPI_COMM_WORLD);
		for (int i = 0; i < num_bins; i++)
		    MPI_Bcast(bins_particle_ids[i], n, MPI_INT, 0, MPI_COMM_WORLD);


        
        for(int i = 0; i < n; i++)
    		printf("myrank = %d step = %d particles[%d].x = %g\n", rank, step, i, particles[i].x);
    	
    	
		for(int i = 0; i < nlocal; i++)
			printf("myrank = %d step = %d local[%d].x = %g\n", rank, step, i, local[i].x);
    

        //
        //  save current step if necessary (slightly different semantics than in other codes)
        //
        if( fsave && (step%SAVEFREQ) == 0 )
            save( fsave, n, particles );
        
        //
        //  compute all forces
        //
        for( int i = 0; i < nlocal; i++ )
            local[i].ax = local[i].ay = 0;
      	
      	  
	  	for (int j = 0; j < num_bins; j++ )
        	go_through_neighbors( local, bins, bins_particle_ids, j );

        //
        //  move particles
        //
        for( int i = 0; i < nlocal; i++ )
            move_and_update( local[i], i, globalIds[i] );


    }

    simulation_time = read_timer( ) - simulation_time;
    
    if( rank == 0 )
		printf( "n = %d, n_procs = %d, simulation time = %g s\n", n, n_proc, simulation_time );
		
	//
	//  release resources
	//
	free( partition_offsets );
	free( partition_sizes );
	free( local );
	free( particles );
	free( bins );

    if( fsave )
    fclose( fsave );
  
    MPI_Finalize( );
    
    return 0;
}
