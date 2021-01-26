#! /usr/bin/gnuplot

# general plot parameters
set terminal png
set datafile separator ","

BIL = 1000000000

# 1.png - Throughput vs # Threads

set title "Synchronized Throughput vs Threads"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Throughput (operations/second)"
set logscale y 10
set output 'lab2b_1.png'

# 1,000 iterations, 1,2,4,8,12,16,24 threads, 1 List, No yield, Sync
plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(BIL/$7) \
	title 'Mutex' with linespoints lc rgb 'red', \
     "< grep 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(BIL/$7) \
	title 'Spin' with linespoints lc rgb 'green'

# 2.png - Time per operation vs # Threads
set title "Mutex  Time per Operation & Wait-for-Lock Time vs Threads"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Time (ns)"
set logscale y 10
set output 'lab2b_2.png'

# 1,000 iterations and 1, 2, 4, 8, 16, 24 threads, 1 List, No yield, Mutex
plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($7) \
	title 'Time per Operation' with linespoints lc rgb 'green', \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($8) \
	title 'Wait-for-Lock Time' with linespoints lc rgb 'red'

# 3.png - Successful # of Iterations vs Threads
set title "Successful # of Iterations vs Threads"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Successful Iterations"
set logscale y 10
set output 'lab2b_3.png'

#Yield ID, 4 lists, 1,4,8,12,16 threads, and 1, 2, 4, 8, 16 iterations, No sync
#Yield ID, 4 lists, 1,4,8,12,16 threads, and 1, 2, 4, 8, 16 iterations, Sync S
#Yield ID, 4 lists, 1,4,8,12,16 threads, and 1, 2, 4, 8, 16 iterations, Sync M
plot \
    "< grep 'list-id-none,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
	title 'Unprotected' with points lc rgb 'red' , \
    "< grep 'list-id-s,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
	title 'Spin' with points lc rgb 'green' , \
    "< grep 'list-id-m,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
	title 'Mutex' with points lc rgb 'blue'
	
# 4.png - aggregated throughput (total operations per second) vs. the number of threads.
#separate curve for each number of lists (1,4,8,16)
set title "Mutex Throughput vs Threads"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:12]
set ylabel "Throughput (operations/second)"
set logscale y 10
set output 'lab2b_4.png'

#No yield, 1000 iterations, 1,2,4,8,12 threads, 1,4,8,16 lists, Sync M
plot \
    "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(BIL/$7) \
	title '1 List' with linespoints lc rgb 'red' , \
    "< grep 'list-none-m,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(BIL/$7) \
	title '4 Lists' with linespoints lc rgb 'green' , \
    "< grep 'list-none-m,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(BIL/$7) \
	title '8 Lists' with linespoints lc rgb 'blue' , \
	"< grep 'list-none-m,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(BIL/$7) \
	title '16 Lists' with linespoints lc rgb 'yellow'
	
# 5.png - aggregated throughput (total operations per second) vs. the number of threads.
#separate curve for each number of lists (1,4,8,16)
set title "Spin Throughput vs Threads"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:12]
set ylabel "Throughput (operations/second)"
set logscale y 10
set output 'lab2b_5.png'

#No yield, 1000 iterations, 1,2,4,8,12 threads, 1,4,8,16 lists, Sync S
plot \
    "< grep 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(BIL/$7) \
	title '1 List' with linespoints lc rgb 'red' , \
    "< grep 'list-none-s,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(BIL/$7) \
	title '4 Lists' with linespoints lc rgb 'green' , \
    "< grep 'list-none-s,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(BIL/$7) \
	title '8 Lists' with linespoints lc rgb 'blue' , \
	"< grep 'list-none-s,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(BIL/$7) \
	title '16 Lists' with linespoints lc rgb 'yellow'