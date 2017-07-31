set terminal latex
set output "log_time.tex"

set key right center

set xlabel "number of particles"
set ylabel  'execution \\time \\in $s$'

set log xy
set xrange [1000:10000]

plot "serial.dat" u 1:2 w linespoints title "Serial", \
	 "pthreads.dat" u 1:2 w linespoints title "Pthreads", \
	 "openmp.dat" u 1:2 w linespoints title "OpenMP"
