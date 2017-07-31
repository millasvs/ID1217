set terminal png
set output "speedup_matrixsum.png"

set title "Speedup of parallel matrixsum"
set key right center

set xlabel "number of rows in matrix"
set ylabel "speedup"


plot "speedup_2.dat" u 1:2 w linespoints title "2 processors", \
	 "speedup_3.dat" u 1:2 w linespoints title "3 processors", \
	 "speedup_4.dat" u 1:2 w linespoints title "4 processors"
