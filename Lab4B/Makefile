build:
	gcc -g3 -Wall -Wextra -lmraa -lm lab4b.c -o lab4b
	
check:
	chmod +x test.sh
	./test.sh
	
clean:
	rm -rf lab4b lab4b-UID.tar.gz
	
dist:
	tar -czvf lab4b-UID.tar.gz lab4b.c test.sh Makefile README