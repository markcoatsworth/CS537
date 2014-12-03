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
response ServerStat(int inum);
response ServerWrite(int inum, char *buffer, int block);
response ServerRead(int inum, int block);
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

/*
**	Main function
*/
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
    	
    	printf("[server] Received data, BytesReceived=%d, IncomingRequest.cmd=%s\n", BytesReceived, IncomingRequest.cmd);
    	if(strcmp(IncomingRequest.cmd, "INIT") == 0)
    	{
			OutgoingResponse = ServerInit();
		}
		else if(strcmp(IncomingRequest.cmd, "LOOKUP") == 0)
		{
			OutgoingResponse = ServerLookup(IncomingRequest.inum, IncomingRequest.name);
		}
		else if(strcmp(IncomingRequest.cmd, "STAT") == 0)
    	{
    		OutgoingResponse = ServerStat(IncomingRequest.inum);
    	}
    	else if(strcmp(IncomingRequest.cmd, "WRITE") == 0)
    	{
			OutgoingResponse = ServerWrite(IncomingRequest.inum, IncomingRequest.block, IncomingRequest.blocknum);
		}
		else if(strcmp(IncomingRequest.cmd, "READ") == 0)
    	{
    		OutgoingResponse = ServerRead(IncomingRequest.inum, IncomingRequest.blocknum);
    	}
    	else if(strcmp(IncomingRequest.cmd, "CREAT") == 0)
    	{
    		OutgoingResponse = ServerRead(IncomingRequest.inum, IncomingRequest.blocknum);
    	}
    	else if(strcmp(IncomingRequest.cmd, "CREAT") == 0)
    	{
    		printf("[server] Received UNLINK message\n");
    	}
    	else if(strcmp(IncomingRequest.cmd, "SHUTDOWN") == 0)
    	{
   			printf("[server] Received SHUTDOWN message\n");
    	}
    	
    	UDP_Write(SocketDescriptor, &UDPSocket, (char*)&OutgoingResponse, sizeof(response));
    }
    
	// Close the UDP connection and exit
	UDP_Close(FileSystemDescriptor);
	
	return 0;
}

/*
**	Initializes an empty file system with a basic root directory
*/
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

/*
**	Initializes the server
*/
response ServerInit()
{
	response ResponseMessage;
	ResponseMessage.rc = 0;
	return ResponseMessage;
}

/*
**	Handles the MFS_Lookup request.
*/
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

/*
**	Handles the MFS_Stat request.
*/
response ServerStat(int inum)
{
	// Local variables
	response ResponseMessage;
	struct dinode Inodes[IPB];
	
	// Retrieve the requested directory inode
	lseek(FileSystemDescriptor, OFFSET_INODES, 0);
	read(FileSystemDescriptor, (void*)Inodes, MFS_BLOCK_SIZE);
	
	// Make sure the inode requested actually exists
	if(inum > 0 && inum < IPB)
	{
		if(Inodes[inum].type > 0)
		{
			ResponseMessage.rc = 0;
			ResponseMessage.stat.type = Inodes[inum].type;
			ResponseMessage.stat.size = Inodes[inum].size;
			return ResponseMessage;					
		}
	}
			
	// If no matching entry was found, return a response message indicating failure
	ResponseMessage.rc = -1;
	return ResponseMessage;

}

response ServerWrite(int inum, char *buffer, int block)
{
	// Local variables
	int BytesWritten;
	response ResponseMessage;
	struct dinode Inodes[IPB];
	
	// Retrieve the requested directory inode
	lseek(FileSystemDescriptor, OFFSET_INODES, 0);
	read(FileSystemDescriptor, (void*)Inodes, MFS_BLOCK_SIZE);
	
	// Make sure the inode at directory pinum is a regular file
	if(Inodes[inum].type == MFS_REGULAR_FILE)
	{
		// Make sure block falls within valid range
		if(block >= 0 && block <= NDIRECT)
		{
			// Verify that the requested block entry is valid
			if(Inodes[inum].addrs[block] != -1)
			{
				// Write the buffer to the address specified by this inode pointer
				lseek(FileSystemDescriptor, Inodes[inum].addrs[block], 0);
				BytesWritten = write(FileSystemDescriptor, buffer, MFS_BLOCK_SIZE);
				
				if(BytesWritten > 0)
				{
					// If write was successful, return a success code
					ResponseMessage.rc = BytesWritten;
					return ResponseMessage;
				}
			}
		}
	}		

	// If we failed any of the conditions, return a response message indicating failure
	ResponseMessage.rc = -1;
	return ResponseMessage;
}

response ServerRead(int inum, int block)
{
	// Local variables
	char* ReadBuffer;
	int BytesRead;
	response ResponseMessage;
	struct dinode Inodes[IPB];
	
	// Allocate the read buffer
	ReadBuffer = (char*) malloc(MFS_BLOCK_SIZE * sizeof(char));
	
	// Retrieve the requested directory inode
	lseek(FileSystemDescriptor, OFFSET_INODES, 0);
	read(FileSystemDescriptor, (void*)Inodes, MFS_BLOCK_SIZE);
	
	// Make sure the inode is valid
	if(Inodes[inum].type > 0)
	{
		// Make sure block falls within valid range
		if(block >= 0 && block <= NDIRECT)
		{
			// Verify that the requested block entry is valid
			if(Inodes[inum].addrs[block] != -1)
			{
				// If we are reading from a regular file, grab the data and return it in the response message
				if(Inodes[inum].type == MFS_REGULAR_FILE)
				{
					lseek(FileSystemDescriptor, Inodes[inum].addrs[block], 0);
					BytesRead = read(FileSystemDescriptor, ReadBuffer, MFS_BLOCK_SIZE);
					
					if(BytesRead > 0)
					{
						// If write was successful, return a success code
						ResponseMessage.rc = BytesRead;
						strcpy(ResponseMessage.block, ReadBuffer);
						return ResponseMessage;
					}
				}
			}
		}
	}
				
	// If we failed any of the conditions, return a response message indicating failure
	ResponseMessage.rc = -1;
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