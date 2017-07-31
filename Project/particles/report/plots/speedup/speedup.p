set terminal latex
set output "speedup.tex"

set title "Speedup parallel applications - 10000 particles"
set key right center

set xlabel "number of processors"
set ylabel "speedup"


plot "openmp_speedup.dat" u 1:2 w linespoints title "openmp", \
	 "pthreads_speedup.dat" u 1:2 w linespoints title "pthreads"
