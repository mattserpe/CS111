build:
	gcc -g3 -Wall -Wextra -pthread -lprofiler SortedList.c lab2_list.c -o lab2_list

tests:
	rm -rf lab2b_list.csv
	chmod +x tests.sh
	./tests.sh
	
profile:
	rm -rf ./raw.gperf profile.out
	LD_PRELOAD=/usr/lib64/libprofiler.so
	CPUPROFILE=./raw.gperf ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text ./lab2_list ./raw.gperf > profile.out
	pprof --list=locking ./lab2_list ./raw.gperf >> profile.out
	rm -rf ./raw.gperf
	
graphs: tests
	chmod +x lab2b_list.gp
	./lab2b_list.gp

dist: build graphs profile clean
	tar -czvf lab2b-UID.tar.gz SortedList.h SortedList.c lab2_list.c \
	Makefile lab2b_list.csv profile.out \
	lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png \
	lab2b_list.gp tests.sh README

clean:
	rm -rf lab2_list lab2b-UID.tar.gz