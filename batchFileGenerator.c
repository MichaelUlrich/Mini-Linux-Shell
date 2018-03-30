#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#define MAX_PATH 512
int main(int argc, char **argv) {
	FILE * fd;
	int i = 0;
	char * path = malloc((sizeof(char) * MAX_PATH)),
	     * command = malloc((sizeof(char) * MAX_PATH));
	
	path[0] = '\0';	
	if(argc > 2) { 							//Error check for too many arguments
		perror("TOO MANY ARGUMENTS PASSED\n");
		exit(EXIT_FAILURE);
	}
	if(argc == 1) { 						//Error check for not enough arguments
		perror("NO ARGUMENTS PASSED\n");
		exit(EXIT_FAILURE);
	}	
	path = argv[1];							//Passing path to local variable
	strcat(path, "batchFile.txt"); 					//Add file name to path
	printf("SAVING BATCH FILE TO: %s\n", path);
	fd  = fopen(path, "wb"); 					//Open path in Read/Write - Create file if it doesn't exist
	if(fd < 0) {							//Error check that fd was able to open bath
                printf("'fd': was un-able to open file entered\n");
	}
	printf("ENTER COMMANDS: (HIT ENTER AFTER EACH COMMAND & TYPE 'DONE' WHEN FINSIHED)\n");
	printf("EXAMPLES PATH: /home/user/Desktop/\n");
	printf("--------------------------------------------------------------------------\n");
	while (1) {
		printf("%i:", i + 1);
		fgets(command, MAX_PATH, stdin);			//Read user input
		if((strcmp("DONE\n\0", command) == 0) || (strcmp("done\n\0", command) == 0)) {
			printf("--------------------------------------------------------------------------\n");
			printf("Commands passed to batch file\n");
			printf("SAVED IN: %s\n", path);
			break;
		}
		fputs(command, fd);					//Pass command to batch file
		i++;
	}
	return 0;
}
