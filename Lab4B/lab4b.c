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

////////////////Globals///////////////////////
int period = 1;
int scaleFlip = 0;
int fd = 9999;
int genReports = 1;

//Sensor variables
mraa_aio_context tempSense;
mraa_gpio_context button;

//Timing variables
struct tm *localTime;
time_t clk = 0;
time_t prevTime = 0;

//Buffered Input
char string[2048];

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
	int tempLen = 4;
	sprintf(tempString, "%d.%1d", tempInt/10, tempInt%10);
	
	if(shutdown){
		sprintf(tempString, "SHUTDOWN");
		tempLen = 8;
	}
		
	//If it is first time or has been one period
	if(shutdown || (genReports && (prevTime == 0 || clk - prevTime >= period))){
		//Report to stdout
		if(fd == 9999 && (shutdown || genReports))
			fprintf(stdout, "%s %s\n", timestamp, tempString);
		
		//Report to log
		if((shutdown || genReports) && fd != 9999){
			write(fd, timestamp, 8);
			write(fd, " ", 1);
			write(fd, tempString, tempLen);
			write(fd, "\n", 1);
		}
		prevTime = clk;
	}
}

void shutDown(){
	report(1);
	exit(0);
}

void inputHandler(char* stdInput, int numCharStdIn){
	//Parse input
	int i;
	int j;

	for(i = 0; i < numCharStdIn; i++){
		if(stdInput[i] != '\n')
			strncat(string, stdInput + i, 1);
		else{
			int length = strlen(string);
			fprintf(stdout, "%s\n", string);
			if(fd != 9999){
				write(fd, string, length);
				write(fd, "\n", 1);
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
	struct pollfd input = { 0, POLLIN, 0};	//Poll stdin
	
	//Poll until exit
	while(1){
		//Print timestamp, temp
		report(0);
		
		//Handle input on stdin
		int x = poll(&input, 1, 0);
		if(x == -1){
			fprintf(stderr, "Error starting polling: %s\n", strerror(errno));
			exit(1);
		}
		//Recieved something from stdin
		if(x && input.revents == POLLIN){
			//Read in input
			char stdInput[2048];
			int numCharStdIn = read(0, &stdInput, 2048);
			
			//Report read error
			if(numCharStdIn == -1){
				fprintf(stderr, "Error reading input: %s\n", strerror(errno));
				exit(1);
			}
			
			//Handle input
			else if(numCharStdIn > 0)
				inputHandler(stdInput, numCharStdIn);
		}
		//Report stdin error
		else if(input.revents == POLLERR){
			fprintf(stderr, "Error polling stdin: %s\n", strerror(errno));
			exit(1);
		}
	}
}

void sensorSetup(){
	//Initialize sensor variables
	tempSense = mraa_aio_init(1);
	if(tempSense == NULL){
		fprintf(stderr, "Error initializing temperature I/O: %s\n", strerror(errno));
		mraa_deinit();
		exit(1);
	}
	button = mraa_gpio_init(60);
	if(button == NULL){
		fprintf(stderr, "Error initializing button I/O: %s\n", strerror(errno));
		mraa_deinit();
		exit(1);
	}
	
	//Set GPIO flow direction
	mraa_gpio_dir(button, MRAA_GPIO_IN);
	
	//When button pressed, shutdown
	mraa_gpio_isr(button, MRAA_GPIO_EDGE_RISING, &shutDown, NULL);
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
			//Undefined Option
			default:
				fprintf(stderr, "Unrecognized Option. Accepted options: --scale=[FC] --period=# --log=filename\n");
				exit(1);
				break;
		}
	}
	
	//Initialize I/O
	sensorSetup();
	
	//Poll stdin
	polling();
	
	//Close variables
	mraa_aio_close(tempSense);
	mraa_gpio_close(button);
	
	exit(0);
}