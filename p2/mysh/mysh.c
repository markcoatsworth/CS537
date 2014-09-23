// Mark Coatsworth
// CS537 Introduction to Operating Systems
// Project 2a
// October 3, 2014

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#define DEBUG 0


int main()
{
	// Shell variables
	char **NextArg;
	char ShellInput[1024];
	char *ShellInputArgs[64];
	int ProcessReturnCode;

	// Main shell loop
	while(1)
	{
		// Display shell prompt
		printf("mysh> ", getpid());
		
		// Read user input from the shell prompt
		// Some of this code is stolen from: http://stackoverflow.com/questions/15539708/passing-an-array-to-execvp-from-the-users-input
		fgets(ShellInput, sizeof(ShellInput), stdin);
	    	
		// Parse the input arguments
		NextArg = ShellInputArgs;
		char *ThisArg = strtok(ShellInput, " \n");
		while (ThisArg != NULL)
    	{
			*NextArg++ = ThisArg;
			ThisArg = strtok(NULL, " \n");
        }
		*NextArg = NULL;
	
		
		// Debug: output the input arguments
		if(DEBUG == 1)
		{
			int ArgIndex = 0;
	        puts("Input arguments:");
	        for (NextArg = ShellInputArgs; *NextArg != 0; NextArg++)
	        {
				printf("[%d] %s\n", ArgIndex, *NextArg);
				ArgIndex++;
			}
		}
			
		// Check for special cases: exit, cd and pwd
		if(strcmp(ShellInputArgs[0], "exit") == 0)
		{
			exit(0);
		}
		else if(strcmp(ShellInputArgs[0], "pwd") == 0)
		{
			char *CurrentDirectory;
			CurrentDirectory = (char *)malloc(sizeof(char) * 512);
			getcwd(CurrentDirectory, 512);
			printf("%s\n", CurrentDirectory);
		}
		// If this not a special case, then fork the process and execute the user input!
		else
		{
			ProcessReturnCode = fork();
	
			// Child process: execute the command + arguments passed in by the user
			if(ProcessReturnCode == 0)
			{
				execvp(ShellInputArgs[0], ShellInputArgs);
				
				// If execvp encounters any error, it will fail and then will process the following code
				perror("Error!\n"); 
				
			}
			// Parent process: do nothing, just wait for the child to end
			else if(ProcessReturnCode > 0)
			{		
				wait();
			}
			// Error: fork failed
			else
			{
				perror("Shell fork failed!");
			}
		}
		
		
	}
	
	return 0;
}
