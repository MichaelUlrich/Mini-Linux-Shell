#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#define MAX_PATH 512
	
int main(int argc, char **argv) {
	FILE * fd;
	int i = 0;
	char * path = malloc((sizeof(char) * MAX_PATH)),
	     * command = malloc((sizeof(char) * MAX_PATH));
	struct stat pathStat;						//Will be used to check if path points to file or directory
	mode_t mode;							//Mode of file, stores value returned of '.st_mode'	
	if(argc > 2) { 							//Error check for too many arguments
		perror("TOO MANY ARGUMENTS PASSED\n");
		exit(EXIT_FAILURE);
	}
	if(argc == 1) { 						//Error check for not enough arguments
		perror("NO ARGUMENTS PASSED\n");
		exit(EXIT_FAILURE);
	}
		
	path = argv[1];							//Passing path to local variable
	if(stat(path, &pathStat) < 0) {
		perror("ERROR GETTING PATH INFO\n");
		exit(EXIT_FAILURE);
	}

	mode = pathStat.st_mode;					//Get mode of file

	if(S_ISDIR(mode)) {	
		printf("DIRECTORY MODE: CREATING FILE IN DIRECTORY\n");		//Path only points to directory, no file - Create file for user to fill with commands
		strcat(path, "batchFile.txt"); 					//Add file name to path
		printf("SAVING BATCH FILE TO: %s\n", path);
		fd  = fopen(path, "wb"); 					//Open path in Read/Write - Create file if it doesn't exist
		if(fd < 0) {							//Error check that fd was able to open bath
        	        printf("'fd': was un-able to open file entered\n");
		}
		printf("ENTER COMMANDS: (HIT ENTER AFTER EACH COMMAND & TYPE 'DONE' WHEN FINSIHED)\n");	
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
	} else if (S_ISREG(mode)) {
		printf("FILE MODE: OPENING FILE IN DIRECTORY\n");		//Path points to file - Will open file for writing
		printf("SAVING BATCH FILE TO: %s\n", path);
                fd  = fopen(path, "wb");                                        //Open path in Read/Write - Truncates file to 0 when opening
                if(fd < 0) {                                                    //Error check that fd was able to open bath
                        printf("'fd': was un-able to open file entered\n");
                }
                printf("ENTER COMMANDS: (HIT ENTER AFTER EACH COMMAND & TYPE 'DONE' WHEN FINSIHED)\n");
                printf("--------------------------------------------------------------------------\n");
                while (1) {
                        printf("%i:", i + 1);
                        fgets(command, MAX_PATH, stdin);                        //Read user input
                        if((strcmp("DONE\n\0", command) == 0) || (strcmp("done\n\0", command) == 0)) {
                                printf("--------------------------------------------------------------------------\n");
                                printf("Commands passed to batch file\n");
                                printf("SAVED IN: %s\n", path);
                                break;
                        }
                        fputs(command, fd);                                     //Pass command to batch file
                        i++;
                }
	}	
	return 0;
}
