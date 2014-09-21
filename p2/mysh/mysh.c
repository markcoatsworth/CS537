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

#define DEBUG 1


int main()
{
	// Shell variables
	char **NextArg;
	char ShellInput[1024];
	char *ShellInputArgs[64];
	int ProcessReturnCode;

	// Should also look at:
	// -- http://stackoverflow.com/questions/14473751/writing-a-shell-how-to-execute-commands

	while(1)
	{
		
		
		//printf("Child process! id=%d\n", getpid());
		// Display shell prompt
		printf("(%d) mysh> ", getpid());
		
		// Read user input from the shell prompt
		// Much of this code is stolen from: http://stackoverflow.com/questions/15539708/passing-an-array-to-execvp-from-the-users-input
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
			
		printf("Now scanning for special cases...\n");
			
		// Check for special cases: exit, cd and pwd
		if(strcmp(ShellInputArgs[0], "exit") == 0)
		{
			printf("Exiting now...\n");
			exit(0);
		}
		else
		{
			ProcessReturnCode = fork();
			printf("Process Return Code = %d\n", ProcessReturnCode);
	
			// Child process: display the prompt, wait for user input, then process it
			if(ProcessReturnCode == 0)
			{
				
				// Start the new process
				execvp(ShellInputArgs[0], ShellInputArgs);
				
		    
			}
			// Parent process: do nothing, just wait for the child to end, then repeat the loop
			else if(ProcessReturnCode > 0)
			{		
					
				wait();
				//printf("Parent process! id=%d\n", getpid());
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
