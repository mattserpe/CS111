#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <errno.h>
#include <string.h>


//////////////Functions//////////////////////

void fileToStdin(){
	int ifd = open(optarg, 0444);
	if(ifd < 0){
		fprintf(stderr, "Could not open input file %s: %s\n", optarg, strerror(errno));
		exit(2);
	}
	if(ifd >= 0){
		close(0);
		dup(ifd);
		close(ifd);
	}
}

void stdoutToFile(){
	int ofd = creat(optarg, 0666);
	if(ofd < 0){
		fprintf(stderr, "Could not create output file %s: %s\n", optarg, strerror(errno));
		exit(3);
	}

	if(ofd >= 0){
		close(1);
		dup(ofd);
		close(ofd);
	}
}

void createSegFault(){
	char *ptr = NULL;
	*ptr = 'a';
	return;
	
}

void seghandler(){
	fprintf(stderr,"Segmentation Fault!\n");
	exit(4);
}

void catch(){
	signal(SIGSEGV, seghandler);
}

void inToOut(){
	char i2o;
	while(read(0, &i2o , sizeof(char)) > 0)
		if(write(1, &i2o , sizeof(char)) < 0)
			fprintf(stderr, "Unable to write to output: %s\n", strerror(errno));
}



/////////////////Main/////////////////////////////

int main(int argc, char* argv[]){
	
	int segFault = 0;
	
	int option_index = 0;

	//Option Struct
	static struct option long_options[] = {
		{"input", required_argument, NULL, 'i'},
		{"output", required_argument, NULL, 'o'},
		{"segfault", no_argument, NULL, 's'},
		{"catch", no_argument, NULL, 'c'},
		{0, 0, 0, 0}
	};

	int option;
	while ((option = getopt_long(argc, argv, "", long_options, &option_index)) >= 0){
		switch (option){
			//Input Redirect
			case 'i':
				fileToStdin();
				break;
			//Output Redirect
			case 'o':
				stdoutToFile();
				break;
			//Segfault
			case 's':
				segFault = 1;
				break;
			//Catch
			case 'c':
				catch();
				break;
			//Undefined Option
			default:
				fprintf(stderr, "Unrecognized Option. Accepted options are --input=filename --output=filename --segfault --catch \n");
				exit(1);
				break;
		}
	}
		
	//Segfault
	if(segFault == 1)
		createSegFault();
	
	inToOut();
	exit(0);
}