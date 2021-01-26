default: lab0.c
	gcc -g3 -Wall -Wextra lab0.c -o lab0
check:
	./check.sh
clean:
	rm -rf lab0 lab0-UID.tar.gz
dist:
	tar -czvf lab0-UID.tar.gz lab0.c Makefile README backtrace.png breakpoint.png check.sh