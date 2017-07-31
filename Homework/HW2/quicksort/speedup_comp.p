set terminal png
set output "speedup_qsort.png"

set title "Speedup of parallel quicksort"
set key right center

set xlabel "number of elements in array"
set ylabel "speedup"

set xrange[10000:8077258]

plot "speedup2.dat" u 1:2 w linespoints title "2 processors", \
	 "speedup3.dat" u 1:2 w linespoints title "3 processors", \
	 "speedup4.dat" u 1:2 w linespoints title "4 processors"
