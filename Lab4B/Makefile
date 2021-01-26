default: tcp tls

tcp: lab4c_tcp.c
	gcc -g3 -Wall -Wextra -lmraa -lm lab4c_tcp.c -o lab4c_tcp
	
tls: lab4c_tls.c
	gcc -g3 -Wall -Wextra -lmraa -lm -lssl -lcrypto lab4c_tls.c -o lab4c_tls
	
clean:
	rm -rf lab4c_tcp lab4c_tls lab4c-UID.tar.gz
	
dist:
	tar -czvf lab4c-UID.tar.gz lab4c_tcp.c lab4c_tls.c Makefile README