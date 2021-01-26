#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <termios.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <zlib.h>

////////////////Globals//////////////////////////
struct termios savedAttributes;
int port = 0;
int sockfd = 0;
int lFlag = 0;
int fdLog = 0;
int cFlag = 0;
z_stream c2s;
z_stream s2c;

//////////////Functions//////////////////////
void pollServerOut(struct pollfd fds[]){
	//If server has output
	if(fds[1].revents == POLLIN){
		char serverOutput[256];
		int numCharServer =	read(fds[1].fd, &serverOutput, 256);
		
		//Read error
		if(numCharServer == -1){
			fprintf(stderr, "Error reading server output: %s\r\n", strerror(errno));
			exit(1);
		}
		//EOF
		if(numCharServer == 0){
			exit(0);
		}
		else if(numCharServer > 0){
			if(cFlag){
				//decompression
				char deCompressedOutput[1024];
				s2c.next_in = (Bytef *)serverOutput;
				s2c.avail_in = (uInt)numCharServer;
				s2c.next_out = (Bytef *)deCompressedOutput;
				s2c.avail_out = 1024;

				do{
					if(inflate(&s2c, Z_SYNC_FLUSH) < 0){
						fprintf(stderr, "Error decompressing in client: %s\r\n", s2c.msg);
						exit(1);
					}
				}while(s2c.avail_in > 0);
				
				int numCharDeCompOut = 1024 - s2c.avail_out;
				//Parse decompressed output
				int i;
				for(i = 0; i < numCharDeCompOut; i++){
					//Make <cr> or <lf> into <cr><lf>
					if(deCompressedOutput[i] == '\r' || deCompressedOutput[i] == '\n'){
						if(write(1, "\r\n", 2) == -1){
							fprintf(stderr, "Error writing to stdout: %s\n", strerror(errno));
							exit(1);
						}
					}
					//No special characters
					else{
						if(write(1, deCompressedOutput + i, 1) == -1){
							fprintf(stderr, "Error writing to stdout: %s\r\n", strerror(errno));
							exit(1);
						}
					}
				}
			}
			//Not compressed
			else{
				//Parse output
				int i;
				for(i = 0; i < numCharServer; i++){
					//Make <cr> or <lf> into <cr><lf>
					if(serverOutput[i] == '\r' || serverOutput[i] == '\n'){
						if(write(1, "\r\n", 2) == -1){
							fprintf(stderr, "Error writing to stdout: %s\n", strerror(errno));
							exit(1);
						}
					}
					//No special character
					else{
						if(write(1, serverOutput + i, 1) == -1){
							fprintf(stderr, "Error writing to stdout: %s\r\n", strerror(errno));
							exit(1);
						}
					}
				}
			}
			//Log recieved bytes (before decompression)
			if(lFlag){
				dprintf(fdLog, "RECEIVED %d bytes: ", numCharServer);
				if(write(fdLog, serverOutput, numCharServer) == -1){
					fprintf(stderr, "Error writing to log file: %s\r\n", strerror(errno));
					exit(1);
				}
				write(fdLog, "\n", 1);
			}
		}
	}
	//If server has hangup or error, it is EOF
	else if(fds[1].revents == POLLERR || fds[1].revents == POLLHUP){
		exit(0);
	}
	
	return;
}


void pollStdIn(struct pollfd fds[]){
	//If stdin has input
	if(fds[0].revents == POLLIN){
		char stdInput[256];
		int numCharStdIn = read(fds[0].fd, &stdInput, 256);
		
		//Read error
		if(numCharStdIn == -1){
			fprintf(stderr, "Error reading input: %s\r\n", strerror(errno));
			exit(1);
		}
		else if(numCharStdIn >= 0){
			if(write(1, stdInput, numCharStdIn) == -1){
					fprintf(stderr, "Error writing to stdout: %s\r\n", strerror(errno));
					exit(1);
			}
			//No compression
			if(!cFlag){
				//Write to socket
				if(write(sockfd, stdInput, numCharStdIn) == -1){
					fprintf(stderr, "Error writing to stdout: %s\r\n", strerror(errno));
					exit(1);
				}
				//Log sent bytes
				if(lFlag){
					dprintf(fdLog, "SENT %d bytes: ", numCharStdIn);
					if(write(fdLog, stdInput, numCharStdIn) == -1){
						fprintf(stderr, "Error writing to log file: %s\r\n", strerror(errno));
						exit(1);
					}
					write(fdLog, "\n", 1);
				}
			}
			//With compression
			else{
				char compressedInput[1024];
				c2s.next_in = (Bytef *)stdInput;
				c2s.avail_in = (uInt)numCharStdIn;
				c2s.next_out = (Bytef *)compressedInput;
				c2s.avail_out = 1024;
				
				do{
					if((deflate(&c2s, Z_SYNC_FLUSH)) < 0){
						fprintf(stderr, "Error compressing in client: %s\r\n", c2s.msg);
						exit(1);
					}
				}while(c2s.avail_in > 0);
				
				int numCharCompIn = 1024 - c2s.avail_out;
				//Write compressed to socket
				if(write(sockfd, compressedInput, numCharCompIn) == -1){
					fprintf(stderr, "Error writing to stdout: %s\r\n", strerror(errno));
					exit(1);
				}
				//Log sent bytes (after compression)
				if(lFlag){
					dprintf(fdLog, "SENT %d bytes: ", numCharCompIn);
					if(write(fdLog, compressedInput, numCharCompIn) == -1){
						fprintf(stderr, "Error writing to stdout: %s\r\n", strerror(errno));
						exit(1);
					}
					write(fdLog, "\n", 1);
				}
			}
		}
	}
	//Report stdin error
	else if(fds[0].revents == POLLERR){
		fprintf(stderr, "Error polling stdin: %s\r\n", strerror(errno));
		exit(1);
	}
	
	return;
}


void polling(){
	//Setup polling
	struct pollfd fds[] = {
		//Keyboard
		{0, POLLIN, 0},
	
		//Server output
		{sockfd, POLLIN, 0}
	};
	
	while(poll(fds, 2, -1) > 0){

		pollStdIn(fds);
		
		pollServerOut(fds);
	}
}

void openSocket(){
	struct sockaddr_in server_address;
	struct hostent* host;
	int address_length = sizeof(server_address);
	
	//Open socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1){
		fprintf(stderr, "Error creating socket from client: %s\r\n", strerror(errno));
		exit(1);
	}
	//Set attributes
	memset((char*)&server_address, 0, address_length);
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port);
	
	host = gethostbyname("localhost");
	
	memcpy((char*)&server_address.sin_addr.s_addr, (char*)host->h_addr, host->h_length);
	
	if(connect(sockfd, (struct sockaddr *) &server_address, address_length) == -1){
		fprintf(stderr, "Error connecting to socket from client: %s\r\n", strerror(errno));
		exit(1);
	}
	
	return;
}

void resetAttributes(){
	if(tcsetattr (0, TCSANOW, &savedAttributes) == -1){
		fprintf(stderr, "Error resetting terminal attributes: %s\r\n", strerror(errno));
		exit(1);
	}
	
	if(cFlag){
		deflateEnd(&c2s);
		inflateEnd(&s2c);
	}
	return;
}

void changeAttributes(){
	//Set new attributes
	struct termios nonCanNoEc;
	if(tcgetattr(0, &nonCanNoEc) == -1){
		fprintf(stderr, "Error getting terminal attributes: %s\n", strerror(errno));
		exit(1);
	}
	nonCanNoEc.c_iflag = ISTRIP;	/* only lower 7 bits	*/
	nonCanNoEc.c_oflag = 0;			/* no processing	*/
	nonCanNoEc.c_lflag = 0;			/* no processing	*/
	
	if(tcsetattr (0, TCSANOW, &nonCanNoEc) == -1){
		fprintf(stderr, "Error setting new terminal attributes: %s\n", strerror(errno));
		exit(1);
	}

	return;
}	

/////////////////Main/////////////////////////////

int main(int argc, char* argv[]){

	int option_index = 0;
	int pFlag = 0;	

	//Option Struct
	static struct option long_options[] = {
		{"port", required_argument, NULL, 'p'},
		{"log", required_argument, NULL, 'l'},
		{"compress", no_argument, NULL, 'c'},
		{0, 0, 0, 0}
	};

	int option;
	while ((option = getopt_long(argc, argv, "", long_options, &option_index)) >= 0){
		switch (option){
			//Port option
			case 'p':
				pFlag = 1;
				port = atoi(optarg); 
				break;
			//Log option
			case 'l':
				lFlag = 1;
				if((fdLog = creat(optarg, S_IRWXU)) == -1){	
					fprintf(stderr, "Error opening/creating log file: %s\n", strerror(errno));
					exit(1);
				}
				break;
			//Compress option
			case 'c':
				cFlag = 1;
				//Initialize compression and decompression
				c2s.zalloc = Z_NULL;
				c2s.zfree = Z_NULL;
				c2s.opaque = Z_NULL;
				s2c.zalloc = Z_NULL;
				s2c.zfree = Z_NULL;
				s2c.opaque = Z_NULL;
				if(deflateInit(&c2s, Z_DEFAULT_COMPRESSION) != Z_OK){
					fprintf(stderr, "Error initializing deflate in client: %s\n", strerror(errno));
					exit(1);
				}
				if(inflateInit(&s2c) != Z_OK){
					fprintf(stderr, "Error initializing inflate in client: %s\n", strerror(errno));
					exit(1);
				}
				break;
			//Undefined Option
			default:
				fprintf(stderr, "Unrecognized Option. Accepted options: --port=port --log=filename --compress\n");
				exit(1);
				break;
		}
	}
	
	//No port provided
	if(!pFlag){
		fprintf(stderr, "No port specified. Please use --port=port to specify port.\n");
		exit(1);
	}
	
	//Save original terminal attributes
	if(tcgetattr(0, &savedAttributes) == -1){
		fprintf(stderr, "Error getting terminal attributes: %s\n", strerror(errno));
		exit(1);
	}
	
	//Reset attributes, close compression at exit
	atexit(resetAttributes);
	
	
	//Change terminal attributes
	changeAttributes();
	
	//Open socket
	openSocket();
	
	//Poll
	polling();
	
	exit(0);
}