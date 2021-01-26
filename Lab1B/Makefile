default: client server

client: lab1b-client.c
	gcc -g3 -Wall -Wextra -lz lab1b-client.c -o lab1b-client 
server: lab1b-server.c
	gcc -g3 -Wall -Wextra -lz lab1b-server.c -o lab1b-server
clean:
	rm -rf lab1b-client lab1b-server lab1b-UID.tar.gz
dist:
	tar -czvf lab1b-UID.tar.gz lab1b-client.c lab1b-server.c Makefile README