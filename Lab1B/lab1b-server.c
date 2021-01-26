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
#include <signal.h>
#include <sys/wait.h>
#include <zlib.h>

////////////////Globals//////////////////////////
int port = 0;
int sock = 0;
int clientSock = 0;
int cFlag= 0;
char* program;
int PID;
int endOfFile = 0;
int shellStatus;
z_stream c2s;
z_stream s2c;

//////////////Functions//////////////////////
void end(){
	if(cFlag){
		inflateEnd(&c2s);
		deflateEnd(&s2c);
	}
}

void shellPollOut(struct pollfd fds[]){
	//If shell has output
	if(fds[1].revents == POLLIN){
		char shellOutput[256];
		int numCharShell =	read(fds[1].fd, &shellOutput, 256);
		
		//Read error
		if(numCharShell == -1){
			fprintf(stderr, "Error reading shell output: %s\n", strerror(errno));
			exit(1);
		}
		//EOF
		if(numCharShell == 0){
			endOfFile = 1;
		}
		else if(numCharShell > 0){
			if(cFlag){
			//Compression
				char compressedOutput[1024];
				s2c.next_in = (Bytef *)shellOutput;
				s2c.avail_in = (uInt) numCharShell;
				s2c.next_out = (Bytef *)compressedOutput;
				s2c.avail_out = 1024;
				
				do{
					if(deflate(&s2c, Z_SYNC_FLUSH) < 0){
						fprintf(stderr, "Error compressing in server: %s\r\n", s2c.msg);
						exit(1);
					}
				}while(s2c.avail_in > 0);
				
				int numCharCompOut = 1024 - s2c.avail_out;
				//Write compressed output to sock
				if(write(fds[0].fd, compressedOutput, numCharCompOut) == -1){
					fprintf(stderr, "Error writing to socket: %s\n", strerror(errno));
					exit(1);
				}
			}
			//No Compression
			else{
				if(write(fds[0].fd, shellOutput, numCharShell) == -1){
					fprintf(stderr, "Error writing to socket: %s\n", strerror(errno));
					exit(1);
				}
			}
		}
	}
	//If shell has hangup or error, it is EOF
	else if(fds[1].revents == POLLERR || fds[1].revents == POLLHUP){
		endOfFile = 1;
	}
	
	return;
}

void clientPollIn(struct pollfd fds[], int pipeToChild[]){
	//If socket	has input
	if(fds[0].revents == POLLIN){
		char clientInput[256];
		int numCharClientIn = read(fds[0].fd, &clientInput, 256);
		
		//Read error
		if(numCharClientIn == -1){
			fprintf(stderr, "Error reading input: %s\n", strerror(errno));
			exit(1);
		}
		//End of file
		else if(numCharClientIn == 0){
			if(close(pipeToChild[1]) == -1){
				fprintf(stderr, "Error closing write pipe to shell: %s\n", strerror(errno));
				exit(1);
			}

		}
		else if(numCharClientIn > 0){
			if(cFlag){
				//decompress
				char deCompressedInput[1024];
				c2s.next_in = (Bytef *)clientInput;
				c2s.avail_in = (uInt)numCharClientIn;
				c2s.next_out = (Bytef *)deCompressedInput;
				c2s.avail_out = 1024;

				do{
					if(inflate(&c2s, Z_SYNC_FLUSH) < 0){
						fprintf(stderr, "Error decompressing in server: %s\r\n", c2s.msg);
						exit(1);
					}
				}while(c2s.avail_in > 0);
				
				int numCharDeCompIn = 1024 - c2s.avail_out;
				int i;
				for(i = 0; i < numCharDeCompIn; i++){
					//Handle <cr>, <lf>
					if(deCompressedInput[i] == '\r' || deCompressedInput[i] == '\n' ){
						if(write(pipeToChild[1], "\n", 1) == -1){
							fprintf(stderr, "Error writing to shell: %s\n", strerror(errno));
							exit(1);				
						}
					}
					//Received EXT
					else if(deCompressedInput[i] == '\003'){
						if(kill(PID, SIGINT) == -1){
							fprintf(stderr, "Error sending SIGINT to shell: %s\n", strerror(errno));
							exit(1);
						}
					}
					//Received EOF
					else if(deCompressedInput[i] == '\004'){
						endOfFile = 1;
					}
					//Write to pipe
					else{
						if(write(pipeToChild[1], deCompressedInput + i, 1) == -1){
							fprintf(stderr, "Error writing to shell: %s\n", strerror(errno));
							exit(1);				
						}
					}
				}
			}
			else{
				int i;
				for(i = 0; i < numCharClientIn; i++){
					//Handle <cr>, <lf>
					if(clientInput[i] == '\r' || clientInput[i] == '\n' ){
						if(write(pipeToChild[1], "\n", 1) == -1){
							fprintf(stderr, "Error writing to shell: %s\n", strerror(errno));
							exit(1);				
						}
					}
					//Received EXT
					else if(clientInput[i] == '\003'){
						if(kill(PID, SIGINT) == -1){
							fprintf(stderr, "Error sending SIGINT to shell: %s\n", strerror(errno));
							exit(1);
						}
					}
					//Received EOF
					else if(clientInput[i] == '\004'){
						endOfFile = 1;
					}
					//Write to pipe
					else{
						if(write(pipeToChild[1], clientInput + i, 1) == -1){
							fprintf(stderr, "Error writing to shell: %s\n", strerror(errno));
							exit(1);				
						}
					}
				}
			}
		}
	}
	//Report socket error
	else if(fds[0].revents == POLLERR){
		fprintf(stderr, "Error polling socket: %s\n", strerror(errno));
		exit(1);
	}
}

void polling(int pipeToChild[], int pipeToParent[]){
	//Setup polling
	struct pollfd fds[] = {
		//Clent input
		{clientSock, POLLIN, 0},
	
		//Shell output
		{pipeToParent[0], POLLIN, 0}
	};
	
	//Poll until EOF received
	while(!endOfFile){
		//Begin polling with no timeout and error check
		if(poll(fds, 2, -1) == -1){
			fprintf(stderr, "Error starting polling: %s\n", strerror(errno));
			exit(1);
		}
		else{
				clientPollIn(fds, pipeToChild);
				
				shellPollOut(fds);
		}
	}
	//After EOF or SIGPIPE received
	if(close(pipeToChild[1]) == -1){
		fprintf(stderr, "Error closing write pipe to shell: %s\n", strerror(errno));
		exit(1);
	}
	if(close(pipeToParent[0]) == -1){
		fprintf(stderr, "Error closing pipe: %s\n", strerror(errno));
		exit(1);
	}
	
	//Get exit status
	if(waitpid(PID, &shellStatus, 0) == -1){
		fprintf(stderr, "Error getting exit status: %s\n", strerror(errno));
		exit(1);
	}
		
	//Report exit status
	if(WIFEXITED(shellStatus)){
		fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\r\n", WTERMSIG(shellStatus), WEXITSTATUS(shellStatus));
	}
	
	if(close(sock) == -1){
		fprintf(stderr, "Error closing socket: %s\n", strerror(errno));
		exit(1);
	}
	if(close(clientSock) == -1){
		fprintf(stderr, "Error closing socket: %s\n", strerror(errno));
		exit(1);
	}
	
	return;
}

void parentProcess(int pipeToChild[], int pipeToParent[]){
	//Define parent process
	if(PID > 0){
		//Close unnecessary pipes
		if(close(pipeToChild[0]) == -1){
			fprintf(stderr, "Error closing pipe: %s\n", strerror(errno));
			exit(1);
		}
		if(close(pipeToParent[1]) == -1){
			fprintf(stderr, "Error closing pipe: %s\n", strerror(errno));
			exit(1);
		}
		
		polling(pipeToChild, pipeToParent);
	}
	return;
}

void childProcess(int pipeToChild[], int pipeToParent[]){
	//Define child process
	if(PID == 0){
		//Close unnecessary pipes
		if(close(pipeToChild[1]) == -1){
			fprintf(stderr, "Error closing pipe: %s\n", strerror(errno));
			exit(1);
		}
		if(close(pipeToParent[0]) == -1){
			fprintf(stderr, "Error closing pipe: %s\n", strerror(errno));
			exit(1);
		}
		
		//Perform input and output redirection and check for errors
		if(close(0) == -1){
			fprintf(stderr, "Error closing pipe: %s\n", strerror(errno));
			exit(1);
		}
		if(dup(pipeToChild[0]) == -1){
			fprintf(stderr, "Error duplicating pipe: %s\n", strerror(errno));
			exit(1);
		}
		if(close(1) == -1){
			fprintf(stderr, "Error closing pipe: %s\n", strerror(errno));
			exit(1);
		}
		if(dup(pipeToParent[1]) == -1){
			fprintf(stderr, "Error duplicating pipe: %s\n", strerror(errno));
			exit(1);
		}
		if(close(2) == -1){
			fprintf(stderr, "Error closing pipe: %s\n", strerror(errno));
			exit(1);
		}
		if(dup(pipeToParent[1]) == -1){
			fprintf(stderr, "Error duplicating pipe: %s\n", strerror(errno));
			exit(1);
		}
		if(close(pipeToChild[0]) == -1){
			fprintf(stderr, "Error closing pipe: %s\n", strerror(errno));
			exit(1);
		}
		if(close(pipeToParent[1]) == -1){
			fprintf(stderr, "Error closing pipe: %s\n", strerror(errno));
			exit(1);
		}
		
		//Start shell and error check
		if(execl(program, program, (char*) NULL) == -1){
			fprintf(stderr, "Error starting shell: %s\n", strerror(errno));
			exit(1);
		}
	}
	return;
}

void shellFunc(){
	
	//Define pipes
	int pipeToChild[2];
	int pipeToParent[2];
	
	//Make pipes and error check
	if(pipe(pipeToChild) != 0){
		fprintf(stderr, "Error creating pipe to shell: %s\n", strerror(errno));
		exit(1);
	}
	if(pipe(pipeToParent) != 0){
		fprintf(stderr, "Error creating pipe from shell: %s\n", strerror(errno));
		exit(1);
	}
	
	//Create child
	PID = fork();
	
	//Error check
	if(PID == -1){
		fprintf(stderr, "Error forking child process: %s\n", strerror(errno));
		exit(1);
	}
	
	childProcess(pipeToChild, pipeToParent);
	parentProcess(pipeToChild, pipeToParent);
	
	exit(0);
}

void sigPipeHandler(){
	endOfFile = 1;
}

void openSocket(){
	struct sockaddr_in server_address, client_address;
	int address_length = sizeof(server_address);
	unsigned int client_length = sizeof(client_address);
	
	//Open Socket
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == -1){
		fprintf(stderr, "Error creating socket from server: %s\r\n", strerror(errno));
		exit(1);
	}
	
	//Set attributes
	memset((char*)&server_address, 0, address_length);
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port);
	server_address.sin_addr.s_addr = INADDR_ANY;
	
	if(bind(sock, (struct sockaddr *) &server_address, address_length) == -1){
		fprintf(stderr, "Error binding: %s\r\n", strerror(errno));
		exit(1);
	}
	if(listen(sock, 10) == -1){
		fprintf(stderr, "Error listening: %s\r\n", strerror(errno));
		exit(1);
	}
	clientSock = accept(sock, (struct sockaddr *) &client_address, &client_length);
	if(clientSock == -1){
		fprintf(stderr, "Error accepting: %s\r\n", strerror(errno));
		exit(1);
	}
	return;
}

/////////////////Main/////////////////////////////

int main(int argc, char* argv[]){

	int option_index = 0;
	int shell = 0;
	int pFlag = 0;
	
	//Option Struct
	static struct option long_options[] = {
		{"shell", required_argument, NULL, 's'},
		{"port", required_argument, NULL, 'p'},
		{"compress", no_argument, NULL, 'c'},
		{0, 0, 0, 0}
	};

	int option;
	while ((option = getopt_long(argc, argv, "", long_options, &option_index)) >= 0){
		switch (option){
			//Shell option
			case 's':
				shell = 1;
				program = optarg; 
				break;
			//Port option
			case 'p':
				pFlag = 1;
				port = atoi(optarg); 
				break;
			//Compress option
			case 'c':
				cFlag= 1;
				//Initialize compression and decompression
				c2s.zalloc = Z_NULL;
				c2s.zfree = Z_NULL;
				c2s.opaque = Z_NULL;
				s2c.zalloc = Z_NULL;
				s2c.zfree = Z_NULL;
				s2c.opaque = Z_NULL;
				if(deflateInit(&s2c, Z_DEFAULT_COMPRESSION) != Z_OK){
					fprintf(stderr, "Error initializing deflate in server: %s\n", strerror(errno));
					exit(1);
				}
				if(inflateInit(&c2s) != Z_OK){
					fprintf(stderr, "Error initializing inflate in server: %s\n", strerror(errno));
					exit(1);
				}
				break;
			//Undefined Option
			default:
				fprintf(stderr, "Unrecognized Option. Accepted options: --shell=program --port=port --compress\n");
				exit(1);
				break;
		}
	}
	
	//No port provided
	if(!pFlag){
		fprintf(stderr, "No port specified. Please use --port=port to specify port.\n");
		exit(1);
	}
	
	//Shell mode
	if(!shell){
		fprintf(stderr, "No shell program specified. Please use --shell=program to specify shell program.\n");
		exit(1);
	}
	else{
		//Close copression at exit
		atexit(end);
		
		//Open socket
		openSocket();
		
		//Create SIGPIPE handler
		signal(SIGPIPE, sigPipeHandler);	
			
		//Create shell
		shellFunc();
	}

	exit(0);
}	