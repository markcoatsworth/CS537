#include <stdio.h>
#include <stdlib.h>

#include "mfs.h"
#include "udp.h"

/*
**	Global variables
*/

char *FileSystemImageFile;
int PortNumber;

/*
**	Get command line arguments and save them to relevant pointers. Exit if any problems.
*/
void getargs(int argc, char *argv[])
{
    if (argc != 3) 
    {
		fprintf(stderr, "Usage: server [portnum] [file-system-image]\n");
		exit(1);
    }
    PortNumber = atoi(argv[1]);
    FileSystemImageFile = argv[2];
}


int main(int argc, char *argv[])
{
	// Define local variables
	int FileSystemDescriptor;
	
	// Verify + store command line arguments
    getargs(argc, argv);
    
    // Open the server on the specified port
    //UDP_Open(PortNumber);
    
    // Open the file system image. If it does not exist, create it and initialize it.
    FileSystemDescriptor = open(FileSystemImageFile, O_RDWR, S_IRWXU | S_IRUSR);
    if(FileSystemDescriptor < 0)
    {
    	printf("[server] File system image does not exist, creating it\n");
    	FileSystemDescriptor = open(FileSystemImageFile, O_RDWR | O_CREAT, S_IRWXU | S_IRUSR);
    	printf("[server] Initializing file system image ...\n");
    	
    	
    }
    
    // Enter the main server loop
    /*
    while(1)
    {
    	UDP_Read(
    }
	*/
	
	return 0;
}