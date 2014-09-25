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
	char **NextPipedArg;
	char OutputFileName[256];
	char *PipedArgs[64];
	char ShellInput[1024];
	char *ShellInputArgs[64];
	int IsAppendRedirected;
	int IsOverwritedRedirected;
	int IsPiped;
	int StandardInputDescriptor = dup(STDIN_FILENO);
	int StandardOutputDescriptor = dup(STDOUT_FILENO);
	int PipeDescriptor[2];
	int PipeProcessReturnCode;
	int ProcessReturnCode;

	// Main shell loop
	while(1)
	{
		// Reset all flags and other variables
		IsAppendRedirected = 0;
		IsOverwritedRedirected = 0;
		IsPiped = 0;
	
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
			
		// Check for special characters: overwrite redirection (">"), append redirection (">>"), and pipes ("|").
		int ArgIndex = 0;
        for (NextArg = ShellInputArgs; *NextArg != 0; NextArg++)
        {
			if(strcmp(*NextArg, ">") == 0)
			{
				IsOverwritedRedirected = 1;
				// Set NextArg to NULL, so that only the tokens before the pipe will get executed
				*NextArg = NULL;
				// Now set the output filename, and then break out of the loop
				NextArg++;
				strcpy(OutputFileName, *NextArg);
				break;
			}
			else if(strcmp(*NextArg, ">>") == 0)
			{
				IsAppendRedirected = 1;
				// Set NextArg to NULL, so that only the tokens before the pipe will get executed
				*NextArg = NULL;
				// Now set the output filename, and then break out of the loop
				NextArg++;
				strcpy(OutputFileName, *NextArg);
				break;
			}
			else if(strcmp(*NextArg, "|") == 0)
			{
				IsPiped = 1;
				// On piped input, we first split the input array into two separate arrays
				// First set NextArg to NULL, so that only the tokens before the pipe will get executed
				*NextArg = NULL;
				// Now set up the PipedInput array, which will contain all arguments after the pipe
				NextArg++;
				for (NextPipedArg = PipedArgs; *NextArg != 0; NextArg++)
        		{
					*NextPipedArg++ = *NextArg;	
	        	}
				*NextPipedArg = NULL;
				break;
			}
			ArgIndex++;

		}

		// Debug: output the piped arguments
		if(DEBUG == 1 && IsPiped == 1)
		{
			puts("Pipe detected! Arguments after pipe split:");
			int ArgIndex = 0;
			puts("Input arguments:");
			for (NextArg = ShellInputArgs; *NextArg != 0; NextArg++)
	        {
				printf("[%d] %s\n", ArgIndex, *NextArg);
				ArgIndex++;
			}	        	
			puts("Piped arguments:");
			ArgIndex = 0;
	        for (NextArg = PipedArgs; *NextArg != 0; NextArg++)
	        {
				printf("[%d] %s\n", ArgIndex, *NextArg);
				ArgIndex++;
			}
		}
		
		// Check for built in commands: exit, cd and pwd
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
		else if(strcmp(ShellInputArgs[0], "cd") == 0)
		{
			if(ShellInputArgs[1] != NULL)
			{
				chdir(ShellInputArgs[1]);
			}
			else
			{
				chdir("~");
			}
		}
		// If this not a built in command,  check for special characters, then fork the input!
		else
		{
			// If any special characters were found, redirect output accordingly
			if(IsOverwritedRedirected == 1)
			{
				// Close the file descriptor associated with standard output
				close(1); 
				// Set the output file descriptor, in create mode, set to 644 permissions
				int OutputFileDescriptor = open(OutputFileName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
				// Now open the output file descriptor
				dup2(OutputFileDescriptor, 1);
			}
			else if(IsAppendRedirected == 1)
			{
				// Close the file descriptor associated with standard output
				close(1);
				// Set the output file descriptor, in append mode, set to 644 permissions
				int OutputFileDescriptor = open(OutputFileName, O_APPEND | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
				// Now open the output file descriptor
				dup2(OutputFileDescriptor, 1);
			}
			else if(IsPiped == 1)
			{
				if(pipe(PipeDescriptor) == 1)
				{
					perror("Error: pipe failed\n");
				}
			}
		
			// At this point, fork the process
			ProcessReturnCode = fork();

			// Child process: execute the command + arguments passed in by the user
			if(ProcessReturnCode == 0)
			{
				// If the process was piped, redirect child process output to the pipe
				if(IsPiped == 1)
				{
					printf("Child process, redirected output to pipe\n");
					close(1);
					dup(PipeDescriptor[1]);
					//close(PipeDescriptor[0]);
					//close(PipeDescriptor[1]);
				}
			
				// Execute the shell input
				execvp(ShellInputArgs[0], ShellInputArgs);
				
				// If execvp encounters any error, it will fail and then will process the following code
				perror("Error!\n"); 
				
			}
			// Parent process: wait for the child to end
			else if(ProcessReturnCode > 0)
			{		
				wait();
				
				// If this process was piped, we now have to run to redirect the output to the piped input
				if(IsPiped == 1)
				{
					printf("Return from pipe, waiting...\n");
					// Redirect standard input from the pipe
					close(0);
					dup(PipeDescriptor[0]);
					// Now execute the piped process, using redirected input
					execvp(PipedArgs[0], PipedArgs);
					//close(PipeDescriptor[0]);
					//close(PipeDescriptor[1]);
				}
			}
			// Error: fork failed
			else
			{
				perror("Shell fork failed!");
			}

		}

		// Finally, reset standard input and output		
		dup2(StandardInputDescriptor, 0);			
		dup2(StandardOutputDescriptor, 1);		
		
	}
	
	return 0;
}