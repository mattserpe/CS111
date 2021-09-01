////////TCP////////
//Host Name: lever.cs.ucla.edu
//Port #: 18000
//Server Status URL: http://lever.cs.ucla.edu/TCP_SERVER/index.html

////////TLS////////
//Host Name: lever.cs.ucla.edu
//Port #: 19000
//Server Status URL: http://lever.cs.ucla.edu/TLS_SERVER/index.html

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <mraa.h>
#include <poll.h>
#include <signal.h>
#include <time.h>
#include <sys/socket.h>
#include <netdb.h> 

////////////////Globals///////////////////////
int period = 1;
int scaleFlip = 0;
int fd = 9999;
int genReports = 1;
char* id = "";
char* host = "";
int port = -1;

//Sensor variables
mraa_aio_context tempSense;

//Timing variables
struct tm *localTime;
time_t clk = 0;
time_t prevTime = 0;

//Buffered Input
char string[2048];

//TCP
int sockfd;
struct sockaddr_in servaddr;
struct hostent *servhost;


//////////////Functions//////////////////////
float getTemp(){
	int a = mraa_aio_read(tempSense);
	int B = 4275;
	int r = 100000;
	float R = 1023.0/a-1.0;
	R = r*R;
	
	float celsius = 1.0/(log(R/r)/B+1/298.15)-273.15;
	float farenheit = (celsius*1.8) + 32;
	
	if(scaleFlip)
		return celsius;
	else
		return farenheit;
}


void report(int shutdown){
	//Get time
	time(&clk);
	localTime = localtime(&clk);
	int hour = localTime->tm_hour;
	int min = localTime->tm_min;
	int sec = localTime->tm_sec;
	char timestamp[16];
	sprintf(timestamp, "%02d:%02d:%02d", hour, min, sec);
	
	//Get temp
	float temp = getTemp();
	int tempInt = temp * 10;
	char tempString[16];
	//int tempLen = 4;
	sprintf(tempString, "%d.%1d", tempInt/10, tempInt%10);
	
	if(shutdown){
		sprintf(tempString, "SHUTDOWN");
	}
		
	//If it is first time or has been one period
	if(shutdown || (genReports && (prevTime == 0 || clk - prevTime >= period))){
		//Report to socket
		if(shutdown || genReports){
			dprintf(sockfd, "%s %s\n", timestamp, tempString);
		}
		//Report to log
		if((shutdown || genReports) && fd != 9999){
			dprintf(fd, "%s %s\n", timestamp, tempString);
		}
		prevTime = clk;
	}
}

void shutDown(){
	report(1);
	exit(0);
}

void inputHandler(char* socketInput, int numCharSocketIn){
	//Parse input
	int i;
	int j;

	for(i = 0; i < numCharSocketIn; i++){
		if(socketInput[i] != '\n')
			strncat(string, socketInput + i, 1);
		else{
			int length = strlen(string);

			if(fd != 9999){
				dprintf(fd, "%s\n", string);
			}
			
			char string7[8];
			string7[0] = '\0';
			strncpy(string7, string, 7);
			string7[7] = '\0';
			
			if(strcmp(string, "STOP") == 0){
				genReports = 0;
			}
			else if(strcmp(string, "START") == 0){
				genReports = 1;
			}
			else if(strcmp(string, "OFF") == 0){
				shutDown();
			}
			else if(strcmp(string7, "SCALE=F") == 0){
				scaleFlip = 0;
			}
			else if(strcmp(string7, "SCALE=C") == 0){
				scaleFlip = 1;
			}
			else if(strcmp(string7, "PERIOD=") == 0){
				char periodString[16];
				periodString[0] = '\0';
				for(j = 7; j < length; j++){
					if(isdigit(string[j]))
						strncat(periodString, string + j, 1);
				}
				period = atoi(periodString);
			}
			string[0] = '\0';
		}
	}
}

void polling(){
	//Setup polling
	struct pollfd input = {sockfd, POLLIN, 0};	//Poll socket

	//Poll until exit
	while(1){
		//Print timestamp, temp
		report(0);
		
		//Handle input on socket
		int x = poll(&input, 1, 0);
		if(x == -1){
			fprintf(stderr, "Error starting polling: %s\n", strerror(errno));
			exit(1);
		}
		//Recieved something from socket
		if(x && input.revents == POLLIN){
			//Read in input
			char socketInput[2048];
			int numCharSocketIn = read(sockfd, &socketInput, 2048);
			
			//Report read error
			if(numCharSocketIn == -1){
				fprintf(stderr, "Error reading input: %s\n", strerror(errno));
				exit(1);
			}
			
			//Handle input
			else if(numCharSocketIn > 0)
				inputHandler(socketInput, numCharSocketIn);
		}
		//Report socket error
		else if(input.revents == POLLERR){
			fprintf(stderr, "Error polling socket: %s\n", strerror(errno));
			exit(1);
		}
	}
}

void sensorSetup(){
	//Initialize sensor variables
	tempSense = mraa_aio_init(1);
	if(tempSense == NULL){
		fprintf(stderr, "Error initializing temperature I/O\n");
		mraa_deinit();
		exit(1);
	}
}

void openTCP(){
	//Create socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1) { 
		printf("Error creating socket\n");
		exit(2);
	}
	
	//Find host
	servhost = gethostbyname(host);
	if(servhost == NULL){
		 printf("Error finding host %s\n", host);
		exit(1);
	}
	
	bzero(&servaddr, sizeof(servaddr));	
  
	// assign IP, PORT 
	servaddr.sin_family = AF_INET;
	memcpy((char*)&servaddr.sin_addr.s_addr, (char*)servhost->h_addr, servhost->h_length);
	servaddr.sin_port = htons(port);
  
	// connect the client socket to server socket 
	if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
		printf("Error connecting with server\n");
		exit(2);
	}
}

//////////////////Main//////////////////////
int main(int argc, char* argv[]){
	int option_index = 0;
	string[0] = '\0';

	//Option Struct
	static struct option long_options[] = {
		{"scale", required_argument, NULL, 's'},
		{"period", required_argument, NULL, 'p'},
		{"log", required_argument, NULL, 'l'},
		{"id", required_argument, NULL, 'i'},
		{"host", required_argument, NULL, 'h'},
		{0, 0, 0, 0}
	};

	int option;
	while ((option = getopt_long(argc, argv, "", long_options, &option_index)) >= 0){
		switch (option){
			//Scale option
			case 's':
				if(strcmp(optarg, "C") == 0)
					scaleFlip = 1;
				break;
			//Period option
			case 'p':
				period = atoi(optarg);
				break;
			//Log option
			case 'l':
				fd = creat(optarg, S_IRWXU);
				if(fd == -1){
					fprintf(stderr, "Error opening/creating log file: %s\n", strerror(errno));
					exit(1);
				}
				break;
			//ID option
			case 'i':
				id = optarg;
				break;
			//Host option
			case 'h':
				host = optarg;
				break;
			//Undefined Option
			default:
				fprintf(stderr, "Unrecognized Option. Accepted options: --scale=[FC] --period=# --log=filename --id=9-digit-number --host=name or address --log=filename port number\n");
				exit(1);
				break;
		}
	}
	
	//Non-Option Port
	if(optind < argc){
		port = atoi(argv[optind]);
		if(port <= 0){
			fprintf(stderr, "Invalid port number\n");
			exit(1);
		}
	}

	
	if(strlen(id) != 9 || strlen(host) == 0 || fd == 9999 || port == -1){
		fprintf(stderr, "Incorrect/Missing Parameter. Mandatory parameters are --id=9-digit-number --host=name or address --log=filename port number\n");
		exit(1);
	}
	
	//Open TCP Connection
	openTCP();
	
	//Send and log ID
	dprintf(sockfd, "ID=%s\n", id);
	dprintf(fd, "ID=%s\n", id);
	
	//Initialize I/O
	sensorSetup();
	
	//Poll socket
	polling();
	
	//Close variables
	mraa_aio_close(tempSense);
	
	exit(0);
}