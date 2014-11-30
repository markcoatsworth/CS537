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
	//int FileSystemDescriptor;
	
	// Verify + store command line arguments
    getargs(argc, argv);
    
    // Open the server on the specified port
    //UDP_Open(PortNumber);
    
    return 0;
}