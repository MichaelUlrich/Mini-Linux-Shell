#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
/*	
	----------------
	Mini-Shell Modes
	----------------
done	Interactive mode 	- launched with no other commands, provides a prompt for user to enter commands (argc == 1)
	Batch mode 		- launched with batch file containing commands to be executed 			(argc > 1)	

	Example commands 
	----------------	
	'barrier'	- shell stops accepting new commands until current is finished in interactive mound(cannot be executed in background with barrier&)
			  in batch mode shell stops reading commands until all are executed
done	'quit'		- shell stops accepting new commands and exits after executing remaining commands (will finish batch file before exiting)
done	'> /pathname' 	- redirects the output of the command into the file (in notes)
	'coommand&'	- command is executed in the background

	Testing
	---------------
done	Empty Command line			- just presents a new prompt line 		(non-error)
done	Extra white spaces			- process command properly 			(non-error)
	Batch file with no quit command		- properly procces batch file with EOF flag	(non-error)
done	Command does not exist 			- display stderr and continue processing 	(error/continue)
done	Incorrenct number of command arguments 	- display stderr and exit			(error/exit)
	batch file does not exist 		- display stderr and exit			(error/exit)
	
	Goal
	--------------
	Scan all wikipedia articles titled with two capitol letters (ex. AC, AB,...) and track most common used words in articles
		- Requires batch file storing all commands (Generated by seperate program)
	
*/


#define  MAX_COMMAND 512        //Assuming no command will be longer then 512 bytes
enum{FALSE, TRUE};              //Enumerator, TRUE = 1, FALSE = 0
char * builtinsList[] = { 	//Name of built-in functions
	"cd",
	"exit",
	"quit"
};
int BARR_FLAG = FALSE;			//Global variable to track if will force current process to wait for previous to finish
int REDIR_FLAG = FALSE;			//Global variable to track if output will be redirected from stdout to a file
int BACK_FLAG = FALSE;		        //Global variable to track if process will run in backgroundi
int totalBuiltins() {
        return sizeof(builtinsList) / sizeof(char *);	//Return total amount of built-in functions
}
int cdShell(char ** arguments) { 			//Built-in Change Directory function
	int status;
	char  currDir[MAX_COMMAND];
	
	if(arguments[1] == NULL) {			//No argument added
		printf("cd: No argument given to cd\n");
		return 1;
	} else {
		status = chdir(arguments[1]);		//Change directory and check status of change
		if(status != 0) {			//Bad directory
			printf("Error changing directory\n");
			return EXIT_FAILURE;
		}
	}
	if(getcwd(currDir, sizeof(currDir)) != NULL) {	//getcwd error check
                printf("%s\n", currDir);                //Print new directory
	} else {
		printf("error getting cwd\n");
	}
	return 1;
}
int exitShell(char ** arguments) { 	//Built-in exit function
	return 0;	
}
int (*builtinFunctions[]) (char**) = { 	//Addresses of built-in shell functions
        &cdShell,
        &exitShell,
	&exitShell			//quit and exit do the same thing
};
char * readCommand() {
	int i = 0, character;
	char * buffer = malloc((sizeof(char) * MAX_COMMAND));
	if(buffer == 0 ) {
		perror("ERROR ALLOCATING BUFFER(1). EXITING...\n");
		exit(EXIT_FAILURE);
	}

	while(TRUE) {
		character = getchar();									//Read by character
		if(character == EOF || character == '\n') {						//Check for end of file or new line
			buffer[i] = '\0';								//Terminate with NULL character
			return buffer;									//Return terminated buffer of characters
		} else if (character == '&') {								//'&' found in command, will run in background
			printf("BACK_FLAG = TRUE\n");	
			BACK_FLAG = TRUE;								//Will be used trigger background processing in executeComamnd() 
		} else if (character == '>') {								//'>' found int command, will redirect output to file
			printf("REDIR_FLAG = TRUE\n");
			buffer[i] = character;					
			REDIR_FLAG = TRUE;								//Will be used to trigger output redirecting in executeCommand()	
		} else { buffer[i] = character; }							//Add character to buffer
		i++;
	}
}
char ** getArguments(char * command) {		//"**" To return array
	int i = 0;
	char **tokenArray = malloc(MAX_COMMAND * sizeof(char)), *token;
	
	if(tokenArray == 0) {
		perror("ERROR ALLOCATING BUFFER(2). EXITING...\n");
                exit(EXIT_FAILURE);
	}

	token = strtok(command, "\t\n "); 	//Split by tab, new line, or space
	while(token != NULL) {
		if((strcmp(token, "barrier")) == 0) {
			BARR_FLAG = TRUE;	//Barrier was found, will be forced to wait for previous processes to finish
			printf("BARR_FLAG = TRUE\n");
			i++;	
		}
		tokenArray[i] = token; 		//Pass token to token array
		i++;
		token = strtok(NULL, "\t\n ");	//Move through command
	}
	tokenArray[i] = NULL; 			//Terminate token array with null
	return tokenArray;	
}
void redirectOutput(char ** arguments) {
        
	int fd, i = 0;
	char * path;								//Store path for execution
	while(arguments[i] != NULL) {
		if(strcmp(arguments[i],">") == 0) {
			path = arguments[i + 1];				//Get path from arguments
			arguments = realloc(arguments, (sizeof(char) * i));	//Re-size arguments array to exlucde unnecessary arguments 
			arguments[i] = NULL;					//Terminate re-sized array with NULL
			break;
		}	
		i++;
	}
	
	fd  = open(path, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR); 	//Open file in read/write mode. Also creates the file if it does not exist (if it does truncate it's size to 0)
	
	
	if(fd < 0) {								//Error check that fd was able to open bath
                printf("'fd >': was un-able to open file entered\n");
	}
	
	fflush(stdout);
	
	if((dup2(fd, STDOUT_FILENO)) < 0) {					//Error check for re-directing stdout
		perror("Error redirecting stdout\n");
	}
	if((dup2(fd, STDERR_FILENO)) < 0) {					//Error check for re-directing stderr
		perror("Error redirecting stderr\n");    
	}
	if((dup2(fd, STDIN_FILENO)) < 0) {     					//Error check for re-directing stdin
                perror("Error redirecting stdin\n");
        }

	close(fd);	
	printf("going to file");	
        if(execvp(arguments[0], arguments) == -1) {     			//Check if bad command
        	perror("ERROR EXECUTING COMMAND\n");
       	}

}
//Hide output of background command
int executeCommand (char ** arguments) {
	pid_t pid, wait;
	int status;
	int fd;
	pid = fork(); 							//Process ID
		
	if(pid == 0) { 							//Child process	
		if(REDIR_FLAG) {
			redirectOutput(arguments);			//Redirecting outputs from stdout to user designated file
		/*} else if (BACK_FLAG && !BARR_FLAG) {			//Will only run in background if BACK_FLAG = TRUE && BARR_FLAG = FALSE
			fd = open("/dev/null", O_WRONLY);		//Force output to be hidden
			dup2(fd, 1);					//*************************************************************************
			dup2(fd, 2);
			close(fd);
			BACK_FLAG = FALSE;*/
			
		} else {
			if(execvp(arguments[0], arguments) == -1) {	//Bad command
				perror("ERROR EXECUTING COMMAND\n");
			}
			exit(EXIT_FAILURE);
		}
	} else if (pid < 0) { 						//Error forking
		perror("ERROR FORKING\n");
	} else { 							//Parent process
		if(!BACK_FLAG || BARR_FLAG) {				//Foreground Process - Will run in foreground if BACK_FLAG = FALSE || BARR_FLAG = TRUE
			do {	
				wait = waitpid(pid, &status, WUNTRACED);
				REDIR_FLAG = FALSE;
				BARR_FLAG = FALSE;
			} while (!WIFEXITED(status) && !WIFSIGNALED(status));
		} else {						//Background Process - Do not wait for child, run in background
			//Hide output
			BACK_FLAG = FALSE;				//Reset BACK_FLAG to allow next process to potentially run in foreground	
		}		
	}
	return 1;
}
int startCommand( char ** arguments) {
        int i = 0;

        if(arguments[0] == NULL) 					//Empty command  
                return EXIT_FAILURE;
        for(i = 0; i < totalBuiltins(); i++) {				//Run loop for amount of built-in commands
                if(strcmp(arguments[0], builtinsList[i]) == 0) { 	//Check if entered command is a built-in command
                        return(builtinFunctions[i])(arguments); 	//If built-in, return the address of the built-in command with arguments passed
                }
        }
        return executeCommand(arguments); 				//Execute non-built in command
}
void shellLoop() {
	char *command, **arguments;
	int status = 1;

	while(status) {
		printf("shell$>");			//Print shell prompt
		command = readCommand();		//Get command from user input	
		arguments = getArguments(command);	//Seperate arguments from command	
		status = startCommand(arguments);	//Execute function with arguments
	}
}
int main(int argc, char * argv[], char * envp[]) {
	char* batchPath;
	
	if(argc == 1) { 				//Interactive Mode
		shellLoop();
	} else {					//BATCH MODE
		batchPath = argv[1];
		printf("%s\n", batchPath);		//Read commands from file
	}
	return EXIT_SUCCESS;
}	
