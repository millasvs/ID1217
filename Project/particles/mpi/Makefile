#
# Computers with Red Hat Enterprise Linux 5 in the computer room 648, KTH Forum, Kista
#
CC = g++
MPCC =  mpicc 
#-cc=g++
OPENMP = -fopenmp
LIBS = -lm
CFLAGS = -O3

TARGETS = serial pthreads openmp mpi

all:	$(TARGETS)

serial: serial.o common.o
	$(CC) -g -o $@ $(LIBS) serial.o common.o
pthreads: pthreads.o common.o
	$(CC) -g -o $@ $(LIBS) -pthread pthreads.o common.o
openmp: openmp.o common.o
	$(CC) -o $@ $(LIBS) $(OPENMP) openmp.o common.o
mpi: mpi.o common.o
	$(MPCC) -Wall  -g -o $@ $(LIBS) $(MPILIBS) mpi.o common.o

openmp.o: openmp.cpp common.h
	$(CC) -c $(OPENMP) $(CFLAGS) openmp.cpp
serial.o: serial.cpp common.h
	$(CC) -g -c $(CFLAGS) serial.cpp
pthreads.o: pthreads.cpp common.h
	$(CC) -c $(CFLAGS) pthreads.cpp 
mpi.o: mpi.cpp common.h
	$(MPCC) -Wall  -c -g $(CFLAGS) mpi.cpp $(LIBS)
common.o: common.cpp common.h
	$(CC) -Wall -g -c $(CFLAGS) common.cpp $(LIBS)

clean:
	rm -f *.o $(TARGETS)
