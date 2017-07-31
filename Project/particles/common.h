#ifndef __CS267_COMMON_H__
#define __CS267_COMMON_H__

inline int min( int a, int b ) { return a < b ? a : b; }
inline int max( int a, int b ) { return a > b ? a : b; }

//
//  saving parameters
//
const int NSTEPS = 1000;
const int SAVEFREQ = 10;

//
//  tuned constants
//
#define density 0.0005
#define mass    0.01
#define cutoff  0.01
#define min_r   (cutoff/100)
#define dt      0.0005


//
// particle data structure
//
typedef struct 
{
  double x;
  double y;
  double vx;	
  double vy;
  double ax;	
  double ay;
} particle_t;


typedef struct{
	int num_particles;
	int num_neighbors; 
	int* neighbors_ids; 
	int* particle_ids;
} bin_t;



//
//  timing routines
//
double read_timer( );



//
//  simulation routines
//
void set_size( int n );
void init_particles( int n, particle_t *p );	
void apply_force( particle_t &particle, particle_t &neighbor );
void move( particle_t &p );

void go_through_neighbors(particle_t* , bin_t* , int  );
void go_through_neighbors(particle_t* , bin_t* , int , int , int );
void move_and_update( particle_t& , int , int&);
void move_and_update( particle_t& , int );
void init_bins( bin_t*  );
void insert_into_bins(particle_t* , bin_t* , int );
void insert_into_bins(particle_t* , bin_t* , int , int, int);

//
//  I/O routines
//
FILE *open_save( char *filename, int n );
void save( FILE *f, int n, particle_t *p );



//
//  argument processing routines
//
int find_option( int argc, char **argv, const char *option );
int read_int( int argc, char **argv, const char *option, int default_value );
char *read_string( int argc, char **argv, const char *option, char *default_value );

#endif
