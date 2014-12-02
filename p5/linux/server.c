#include <stdio.h>
#include <stdlib.h>

#include "mfs.h"
#include "udp.h"

/*
**	Function declarations
*/

int FileSystemInitialize();
response ServerInit();
response ServerLookup(int pinum, char *name);
response ServerStat(int inum, MFS_Stat_t *m);
response ServerWrite(int inum, char *buffer, int block);
response ServerRead(int inum, char *buffer, int block);
response ServerCreat(int pinum, int type, char *name);
response ServerUnlink(int pinum, char *name);
response ServerShutdown();
void DebugFileSystem();

/*
**	Global variables
*/

int FileSystemDescriptor;
char *FileSystemImageFile;
int PortNumber;

/*
**	Get command line arguments and save them to relevant global variables. Exit if any problems.
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
	int SocketDescriptor;
   	message IncomingRequest;
   	response OutgoingResponse;	
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
    	if(FileSystemInitialize() != 0)
    	{
    		perror("Error initializing file system");
    	}
    }
    
    // Enter the main server loop
	printf("[server] Started UDP file system server\n");
    while(1)
    {
    	int BytesReceived = UDP_Read(SocketDescriptor, &UDPSocket, (char*)&IncomingRequest, sizeof(message));
    	
    	printf("[server] Received data, BytesReceived=%d, IncomingRequest.RequestType=%d\n", BytesReceived, IncomingRequest.type);
    	switch(IncomingRequest.type)
    	{
    		case 0: // INIT
    			printf("[server] Received INIT message\n");
    			break;
    		case 1: // LOOKUP
    			printf("[server] Received LOOKUP message\n");
    			OutgoingResponse = ServerLookup(IncomingRequest.inum, IncomingRequest.name);
    			break;
    		case 2: // STAT
    			printf("[server] Received STAT message\n");
    			break;
    		case 3: // WRITE
    			printf("[server] Received WRITE message\n");
    			break;
    		case 4: // READ
    			printf("[server] Received READ message\n");
    			break;
    		case 5: // CREAT
    			printf("[server] Received CREAT message\n");
    			break;
    		case 6: // UNLINK
    			printf("[server] Received UNLINK message\n");
    			break;
    		case 7: // SHUTDOWN
    			printf("[server] Received SHUTDOWN message\n");
    			break;
    		default:
    			printf("[server] Error, did not receive a standard message type\n");
    			break;
    	}
    	
    	UDP_Write(SocketDescriptor, &UDPSocket, (char*)&OutgoingResponse, sizeof(response));
    }
    
	// Close the UDP connection and exit
	UDP_Close(FileSystemDescriptor);
	
	return 0;
}

int FileSystemInitialize()
{
	// Create data structures for the empty file system
	char EmptyBlock[MFS_BLOCK_SIZE];
	char SuperBlock[MFS_BLOCK_SIZE];
	struct dinode Inodes[64];
	int DataBitmap[1024];
	struct __MFS_DirEnt_t RootDirectory;
	struct __MFS_DirEnt_t RootCurrentDirectory;
	struct __MFS_DirEnt_t RootParentDirectory;
	
	// Create the root directory for the file system, as well as the current "." and parent ".." entries.
   	strcpy(RootDirectory.name, "\0");
	RootDirectory.inum = ROOTINO;
	strcpy(RootCurrentDirectory.name, ".\0");
	RootCurrentDirectory.inum = ROOTINO;
	strcpy(RootParentDirectory.name, "..\0");
	RootParentDirectory.inum = -1;
	
	// Now create an inode for the root directory
	Inodes[0].type = MFS_DIRECTORY;
	Inodes[0].addrs[0] = 0;
	
	// Indicate in the data bitmap that the first data block is taken
	DataBitmap[0] = 1;
	
	// Write data structures to the file system image. Start with the header (ie. non-data) blocks.
	write(FileSystemDescriptor, (const void*)(&EmptyBlock), MFS_BLOCK_SIZE);
	write(FileSystemDescriptor, (const void*)(&SuperBlock), MFS_BLOCK_SIZE);
	write(FileSystemDescriptor, (const void*)(&Inodes), MFS_BLOCK_SIZE);
	write(FileSystemDescriptor, (const void*)(&DataBitmap), MFS_BLOCK_SIZE);
	
	// Now write the first data block. Combine all three directory entries into the same block.
	write(FileSystemDescriptor, (const void*)(&RootDirectory), sizeof(struct __MFS_DirEnt_t));
	write(FileSystemDescriptor, (const void*)(&RootCurrentDirectory), sizeof(struct __MFS_DirEnt_t));
	write(FileSystemDescriptor, (const void*)(&RootParentDirectory), sizeof(struct __MFS_DirEnt_t));
	
	// Now fill the first data block with dummy data
	int i;
	for(i = 1; i <= (64 - 3); i++)
	{
		write(FileSystemDescriptor, (const void*)(&RootParentDirectory), sizeof(struct __MFS_DirEnt_t));
	}
	
	return 0;
}

response ServerInit()
{
	response ResponseMessage;
	return ResponseMessage;
}

response ServerLookup(int pinum, char *name)
{
	// Local variables
	response ResponseMessage;
	struct __MFS_DirEnt_t DirectoryEntries[MFS_BLOCK_SIZE / sizeof(struct __MFS_DirEnt_t)];
	struct dinode Inodes[IPB];
	
	// Retrieve the requested directory inode
	lseek(FileSystemDescriptor, OFFSET_INODES, 0);
	read(FileSystemDescriptor, (void*)Inodes, MFS_BLOCK_SIZE);
	
	// Make sure the inode at directory pinum is a directory
	if(Inodes[pinum].type == MFS_DIRECTORY)
	{
		// Read all directory entries from this inode
		lseek(FileSystemDescriptor, Inodes[pinum].addrs[0], 0);
		read(FileSystemDescriptor, &DirectoryEntries, MFS_BLOCK_SIZE);

		int entry;
		for(entry = 0; entry < MFS_BLOCK_SIZE / sizeof(struct __MFS_DirEnt_t); entry++)
		{
			// If we find a matching entry, set the response code as the inum value, and return the response message
			if(strcmp(DirectoryEntries[entry].name, name) == 0)
			{
				ResponseMessage.rc = DirectoryEntries[entry].inum;
				return ResponseMessage;
			}
		}					
	}
		
	// If no matching entry was found, return a response message indicating failure
	ResponseMessage.rc = -1;
	return ResponseMessage;
}

response ServerStat(int inum, MFS_Stat_t *m)
{
	response ResponseMessage;
	return ResponseMessage;
}

response ServerWrite(int inum, char *buffer, int block)
{
	response ResponseMessage;
	return ResponseMessage;
}

response ServerRead(int inum, char *buffer, int block)
{
	response ResponseMessage;
	return ResponseMessage;
}

response ServerCreat(int pinum, int type, char *name)
{
	response ResponseMessage;
	return ResponseMessage;
}

response ServerUnlink(int pinum, char *name)
{
	response ResponseMessage;
	return ResponseMessage;
}

response ServerShutdown()
{
	response ResponseMessage;
	return ResponseMessage;
}

void DebugFileSystem()
{
	printf("[DebugFileSystem] Starting...\n");
	int i;
	struct __MFS_DirEnt_t DirectoryEntries[MFS_BLOCK_SIZE / sizeof(struct __MFS_DirEnt_t)];
	struct dinode Inodes[IPB];
	
	// Retrieve the requested directory inode
	lseek(FileSystemDescriptor, OFFSET_INODES, 0);
	read(FileSystemDescriptor, (void*)Inodes, MFS_BLOCK_SIZE);
	
	for(i = 0; i < IPB; i ++)
	{
		if(Inodes[i].type > 0)
		{
			printf("[DebugFileSystem] Inodes[%d]: type=%d, size=%d, addrs:\n", i, Inodes[i].type, Inodes[i].size);
			int ptr;
			for(ptr = 0; ptr < NDIRECT+1; ptr++)
			{
				if(Inodes[i].addrs[ptr] != -1)
				{
					printf("\t[%d] %d\n", ptr, Inodes[i].addrs[ptr]);
					if(Inodes[i].type == MFS_DIRECTORY)
					{	
						lseek(FileSystemDescriptor, Inodes[i].addrs[ptr], 0);
						read(FileSystemDescriptor, &DirectoryEntries, MFS_BLOCK_SIZE);
						int entry;
						for(entry = 0; entry < MFS_BLOCK_SIZE / sizeof(struct __MFS_DirEnt_t); entry++)
						{
							if(DirectoryEntries[entry].inum != -1)
							{
								printf("\t\t> %s (%d)\n", DirectoryEntries[entry].name, DirectoryEntries[entry].inum);
							}
						}
						//printf("\tDirectory name: %s, inum: %d\n", DirectoryEntry.name, DirectoryEntries.inum);
					}
					else if(Inodes[i].type == MFS_REGULAR_FILE)
					{
						printf("\tFile Contents:\n");
						char *FileBuffer;
						FileBuffer = (char*) malloc (MFS_BLOCK_SIZE * sizeof(char));
						lseek(FileSystemDescriptor, Inodes[i].addrs[ptr], 0);
						read(FileSystemDescriptor, FileBuffer, MFS_BLOCK_SIZE);
						printf("\t%s\n", FileBuffer);
						free(FileBuffer);
					}
				}
			}
			
		}
	}
}