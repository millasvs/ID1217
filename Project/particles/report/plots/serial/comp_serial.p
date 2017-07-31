set terminal latex
set output "comp.tex"

set key right center

set xlabel "number of particles"
set ylabel 'execution \\time \\in $s$'


plot "serial_initial.dat" u 1:2 w linespoints title "initial serial", \
	 "serial_modified.dat" u 1:2 w linespoints title "modified serial"

