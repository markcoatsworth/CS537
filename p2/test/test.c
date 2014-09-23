#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

int main()
{
	printf("before %d\n", getpid());
	
	// Fork is one of the main ways to create a process in linux
	// But is not very helpful if child does the exact same thing as the parent process
	// Note that the PID of the parent process is returned in the child, and 0 is returned for the parent
	int rc = fork();
	
	// Following code allows us to do different things in child and parent process, depending on the return code
	if(rc == 0)
	{
		// child
		printf("child after %d\n", getpid());
		close(1); // close the file descriptor associated with standard output
		
		// now open a new file
		// Use this for piping and redirectional output
		
		// open("newfile", O_CREAT | O_RDWR);
		
		// Linux will automatically pass it the lowest file descriptor, which in our case is 1
		
		// Here is an even more elegant way of doing this:
		int fd = open("newfile", O_CREAT | O_RDWR);
		dup2(fd, 1);
		
		
		// We want to tell the child to take a bianry, then run it for us
		// The way we do that is using execvp... this is a system process that allows us to run a different process
		// execvp();
		// If it succeeds, our current process will die and then will be replaced by whatever is in execl
		// 		Note: for execvp, the first arugment is the new program that we want it to run
		// 		The second argument is an array of arguments that we are passing to the new program
		// Note we do not pass the number of arguments; instead we just terminate the argument array with NULL
		char *my_argv[4];
		my_argv[0] = strdup("/bin/ls"); // uses a system call -- this will malloc some memory for the string
		my_argv[1] = strdup("-l");
		my_argv[2] = NULL;
		execvp(my_argv[0], my_argv);
		
		// Every file in Linux has three standard descriptors:
		// 1. standard input
		// 2. standard output
		// 3. standard error

		// Following will only be printed if execvp fails; if execvp succeeds, it will take over the process
		printf("Execvp failed!"); 

	}
	else if(rc > 0)
	{
		
		wait(); // parent should wait for the child to be created, run, and return
		printf("parent after %d\n", getpid());
	}
	else
	{
		// If there is an error in linux, it sets a global error variable called errno
		// perror() refers to errno
		perror("fork failed");
	}
	
	
	// after fork() returns
	//printf("after %d\n", getpid());
	
	return 0;
}