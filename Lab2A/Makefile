build: add list

add:
	gcc -g3 -Wall -Wextra -pthread lab2_add.c -o lab2_add

list:
	gcc -g3 -Wall -Wextra -pthread SortedList.c lab2_list.c -o lab2_list

tests:
	rm -rf lab2_add.csv lab2_list.csv
	./tests.sh

graphs: tests
	./lab2_add.gp
	./lab2_list.gp

dist: build graphs clean
	tar -czvf lab2a-UID.tar.gz lab2_add.c SortedList.h SortedList.c lab2_list.c \
	Makefile lab2_add.csv lab2_list.csv \
	lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png \
	lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png \
	lab2_add.gp	lab2_list.gp tests.sh README

clean:
	rm -rf lab2_add lab2_list lab2a-UID.tar.gz