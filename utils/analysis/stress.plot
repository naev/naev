#!/usr/bin/gnuplot

set terminal pngcairo size 600,600 enhanced
set output "stress.png"
set key left
set termoption dashed
set xtics 1
set ytics nomirror
set y2tics
set y2range [0:200]
set grid
plot "stress.dat" using 1:2 w linespoints t "armor over stress speed" axis x1y2,\
   "stress.dat" using 1:3 w linespoints t "armor",\
   "stress.dat" using 1:4 w linespoints t "mass"
