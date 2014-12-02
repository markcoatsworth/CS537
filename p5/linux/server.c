#include <stdio.h>
#include <stdlib.h>

#include "mfs.h"
#include "udp.h"

/*
**	Function declarations
*/

int FileSystemInitialize(int _fsfd);

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
	char *IncomingMessageBuffer = (char*)malloc(100 * sizeof(char));
	int FileSystemDescriptor;
	int SocketDescriptor;
   	struct sockaddr_in UDPSocket;	
	
	// Verify + store command line arguments
    getargs(argc, argv);
    
    // Open the server on the specified port
    SocketDescriptor = UDP_Open(PortNumber);
    if(SocketDescriptor <= 0)
    {
    	perror("Error binding UDP socket");
    	return 1;
    }
    
    // Open the file system image. If it does not exist, create it and initialize it.
    FileSystemDescriptor = open(FileSystemImageFile, O_RDWR, S_IRWXU | S_IRUSR);
    if(FileSystemDescriptor < 0)
    {
    	printf("[server] File system image does not exist, creating it\n");
    	FileSystemDescriptor = open(FileSystemImageFile, O_RDWR | O_CREAT, S_IRWXU | S_IRUSR);
    	if(FileSystemInitialize(FileSystemDescriptor) != 0)
    	{
    		perror("Error initializing file system");
    	}
    }
    
    // Enter the main server loop
	printf("[server] Started UDP file system server\n");
    while(1)
    {
    	int rc = UDP_Read(SocketDescriptor, &UDPSocket, IncomingMessageBuffer, 100);
    	printf("[server] Received data, rc=%d, SocketData=%s\n", rc, IncomingMessageBuffer);
    }
    
	// Close the UDP connection and exit
	UDP_Close(FileSystemDescriptor);
	
	return 0;
}

int FileSystemInitialize(int _fsfd)
{
	// Create data structures for the empty file system
	char EmptyBlock[MFS_BLOCK_SIZE];
	char SuperBlock[MFS_BLOCK_SIZE];
	struct __MFS_Inode_t Inodes[64];
	int DataBitmap[1024];
	struct __MFS_DirEnt_t RootDirectory;
	struct __MFS_DirEnt_t RootCurrentDirectory;
	struct __MFS_DirEnt_t RootParentDirectory;
	
	// Create the root directory for the file system, as well as the current "." and parent ".." entries.
   	strcpy(RootDirectory.name, "\0");
	RootDirectory.inum = 0;
	strcpy(RootCurrentDirectory.name, ".\0");
	RootCurrentDirectory.inum = 0;
	strcpy(RootParentDirectory.name, "..\0");
	RootParentDirectory.inum = -1;
	
	// Now create an inode for the root directory
	Inodes[0].type = MFS_DIRECTORY;
	Inodes[0].dataptr[0] = 0;
	
	// Indicate in the data bitmap that the first data block is taken
	DataBitmap[0] = 1;
	
	// Write data structures to the file system image. Start with the header (ie. non-data) blocks.
	write(_fsfd, (const void*)(&EmptyBlock), MFS_BLOCK_SIZE);
	write(_fsfd, (const void*)(&SuperBlock), MFS_BLOCK_SIZE);
	write(_fsfd, (const void*)(&Inodes), MFS_BLOCK_SIZE);
	write(_fsfd, (const void*)(&DataBitmap), MFS_BLOCK_SIZE);
	
	// Now write the first data block. Combine all three directory entries into the same block.
	write(_fsfd, (const void*)(&RootDirectory), sizeof(struct __MFS_DirEnt_t));
	write(_fsfd, (const void*)(&RootCurrentDirectory), sizeof(struct __MFS_DirEnt_t));
	write(_fsfd, (const void*)(&RootParentDirectory), sizeof(struct __MFS_DirEnt_t));
	
	// Now fill the first data block with dummy data
	int i;
	for(i = 1; i <= (64 - 3); i++)
	{
		write(_fsfd, (const void*)(&RootParentDirectory), sizeof(struct __MFS_DirEnt_t));
	}
	
	return 0;
}