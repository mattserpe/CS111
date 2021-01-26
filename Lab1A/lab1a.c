#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <poll.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>


////////////////Globals//////////////////////////
struct termios savedAttributes;
int endOfFile = 0;
int shellStatus;


//////////////Functions//////////////////////

void resetAttributes(){
	if(tcsetattr (0, TCSANOW, &savedAttributes) == -1){
		fprintf(stderr, "Error resetting terminal attributes: %s\n", strerror(errno));
		exit(1);
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
		else if(numCharShell > 0){
			//Parse output
			int i;
			for(i = 0; i < numCharShell; i++){
				//Received <lf>
				if(shellOutput[i] == '\n'){
					if(write(1, "\r\n", 2) == -1){
						fprintf(stderr, "Error writing to stdout: %s\n", strerror(errno));
						exit(1);
					}
				}
				//Received EOF
				else if(shellOutput[i] == '\004'){
					//write(1, "^D", 2);
					endOfFile = 1;
				}
				else{
					if(write(1, shellOutput + i, 1) == -1){
						fprintf(stderr, "Error writing to stdout: %s\n", strerror(errno));
						exit(1);
					}
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


void stdPollIn(struct pollfd fds[], int pipeToChild[], int PID){
	//If stdin has input
	if(fds[0].revents == POLLIN){
		char stdInput[256];
		int numCharStdIn = read(fds[0].fd, &stdInput, 256);
		
		//Read error
		if(numCharStdIn == -1){
			fprintf(stderr, "Error reading input: %s\n", strerror(errno));
			exit(1);
		}
		else if(numCharStdIn > 0){
			//Parse input
			int i;
			for(i = 0; i < numCharStdIn; i++){
				//Handle <cr>, <lf>
				if(stdInput[i] == '\r' || stdInput[i] == '\n' ){
					if(write(1, "\r\n", 2) == -1){
						fprintf(stderr, "Error writing to stdout: %s\n", strerror(errno));
						exit(1);				
					}
					if(write(pipeToChild[1], "\n", 1) == -1){
						fprintf(stderr, "Error writing to shell: %s\n", strerror(errno));
						exit(1);				
					}
				}
				//Received EXT
				else if(stdInput[i] == '\003'){
					if(write(1, "^C", 2) == -1){
						fprintf(stderr, "Error writing to stdout: %s\n", strerror(errno));
						exit(1);				
					}
					if(kill(PID, SIGINT) == -1){
						fprintf(stderr, "Error sending SIGINT to shell: %s\n", strerror(errno));
						exit(1);
					}
				}
				//Received EOF
				else if(stdInput[i] == '\004'){
					if(write(1, "^D", 2) == -1){
						fprintf(stderr, "Error writing to stdout: %s\n", strerror(errno));
						exit(1);				
					}
					endOfFile = 1;
				}
				//Write to stdout and pipe
				else{
					if(write(1, stdInput + i, 1) == -1){
						fprintf(stderr, "Error writing to stdout: %s\n", strerror(errno));
						exit(1);
					}
					if(write(pipeToChild[1], stdInput + i, 1) == -1){
						fprintf(stderr, "Error writing to shell: %s\n", strerror(errno));
						exit(1);				
					}
				}
			}
		}
	}
	//Report stdin error
	else if(fds[0].revents == POLLERR){
		fprintf(stderr, "Error polling stdin: %s\n", strerror(errno));
		exit(1);
	}
}


void polling(int pipeToChild[], int pipeToParent[], int PID){
	//Setup polling
	struct pollfd fds[] = {
		//Keyboard
		{0, POLLIN, 0},
	
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
					stdPollIn(fds, pipeToChild, PID);
					
					shellPollOut(fds);
			}
		}
	//After EOF received
		if(close(pipeToChild[1]) == -1){
			fprintf(stderr, "Error closing pipe: %s\n", strerror(errno));
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
	return;
}


void parentProcess(int pipeToChild[], int pipeToParent[], int PID){
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
		
/* 	struct termios parentAttributes;
	if(tcgetattr(0, &parentAttributes) == -1){
		fprintf(stderr, "Error getting parent terminal attributes\n");
		exit(1);
	}
	fprintf(stdout, "Parent Attributes: IFLAG:%u OFLAG:%u LFLAG:%u\r\n", parentAttributes.c_iflag, parentAttributes.c_oflag, parentAttributes.c_lflag); */
		
		polling(pipeToChild, pipeToParent, PID);
	}
	return;
}


void childProcess(int pipeToChild[], int pipeToParent[], int PID, char* program){
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
		
		/* 	struct termios childAttributes;
		if(tcgetattr(0, &childAttributes) == -1){
			fprintf(stderr, "Error getting child terminal attributes\n %s", strerror(errno));
			//exit(1);
		}
		fprintf(stdout, "Child Attributes: IFLAG:%u OFLAG:%u LFLAG:%u\r\n", childAttributes.c_iflag, childAttributes.c_oflag, childAttributes.c_lflag); */
		
		//Start shell and error check
		if(execl(program, program, (char*) NULL) == -1){
			fprintf(stderr, "Error starting shell: %s\n", strerror(errno));
			exit(1);
		}
	}
	return;
}


void shellFunc(char* program){
	
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
	int PID = fork();
	
	//Error check
	if(PID == -1){
		fprintf(stderr, "Error forking child process: %s\n", strerror(errno));
		exit(1);
	}
	
	childProcess(pipeToChild, pipeToParent, PID, program);
	parentProcess(pipeToChild, pipeToParent, PID);
	
	exit(0);
}

void sigPipeHandler(){
	endOfFile = 1;
}

/////////////////Main/////////////////////////////

int main(int argc, char* argv[]){

	int option_index = 0;
	int shell = 0;
	char* program;

	//Option Struct
	static struct option long_options[] = {
		{"shell", required_argument, NULL, 's'},
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
			//Undefined Option
			default:
				fprintf(stderr, "Unrecognized Option. Accepted option: --shell=program\n");
				exit(1);
				break;
		}
	}
	
	//Save original terminal attributes
	if(tcgetattr(0, &savedAttributes) == -1){
		fprintf(stderr, "Error getting terminal attributes: %s\n", strerror(errno));
		exit(1);
	}
	
	//Reset attributes at exit
	atexit(resetAttributes);
	
	//fprintf(stdout, "Starting Attributes: IFLAG:%u OFLAG:%u LFLAG:%u\n", savedAttributes.c_iflag, savedAttributes.c_oflag, savedAttributes.c_lflag);
	
	//Change terminal attributes
	changeAttributes();
	
/* 	struct termios newAttributes;
	if(tcgetattr(0, &newAttributes) == -1){
		fprintf(stderr, "Error getting terminal attributes\n");
		exit(1);
	}
	fprintf(stdout, "New Attributes: IFLAG:%u OFLAG:%u LFLAG:%u\r\n", newAttributes.c_iflag, newAttributes.c_oflag, newAttributes.c_lflag); */
	
	//Shell mode
	if(shell){
		//Create SIGPIPE handler
		signal(SIGPIPE, sigPipeHandler);
			
		//Create shell
		shellFunc(program);
	}
	
	//Read from keyboard, write to stdout, perform <cr><lf> replacements
	char buf;
	
	//Read until EOF and write to stdout one character at a time
	while(read(0, &buf, 256) != -1 && buf != '\004'){
		if(buf != '\r' && buf != '\n'){
			if(write(1, &buf, 1) == -1){
				fprintf(stderr, "Error writing to stdout: %s\n", strerror(errno));
				exit(1);
			}	
		}
		else
			if(write(1, "\r\n", 2) == -1){
				fprintf(stderr, "Error writing to stdout: %s\n", strerror(errno));
				exit(1);				
			}
	}

	//Read error
	if(buf != '\004'){
		fprintf(stderr, "Error reading from stdin: %s\n", strerror(errno));
		exit(1);
	}
	//Received EOF
	else{
		if(write(1, "^D", 2) == -1){
			fprintf(stderr, "Error writing to stdout: %s\n", strerror(errno));
			exit(1);				
		}
	}
	
	exit(0);
}