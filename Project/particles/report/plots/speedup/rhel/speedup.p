set terminal latex
set output "speedup_rhel.tex"

set title "Speedup parallel applications - 10000 particles"
set key right center

set xlabel "number of processors"
set ylabel "speedup"


plot "speedup_openmp.dat" u 1:2 w linespoints title "OpenMP", \
	 "speedup_pthreads.dat" u 1:2 w linespoints title "Pthreads"
