#include <stdio.h>

#include "mfs.h"
#include "udp.h"

/*
**	Global variables
*/

//int ServerSocket;

int MFS_Init(char *hostname, int port)
{
	printf("[MFS_Init] hostname=%s, port=%d\n", hostname, port);

	// Declare variables
	char *InitMessage;
	int SocketDescriptor = UDP_Open(0);
	struct sockaddr_in UDPSocket;

	// Fill the UDPSocket object
	if(UDP_FillSockAddr(&UDPSocket, hostname, port) != 0)
	{
		printf("[MFS_Init] Error filling UDPSocket object\n");
	}
	
	// Send the initialize message
	InitMessage = (char*) malloc(100 * sizeof(char));
	strcpy(InitMessage, "hello!");
	UDP_Write(SocketDescriptor, &UDPSocket, InitMessage, 100);
	
	return 0;
}

int MFS_Lookup(int pinum, char *name)
{
	printf("[MFS_Lookup]\n");
	return 0;
}

int MFS_Stat(int inum, MFS_Stat_t *m)
{
	printf("[MFS_Stat]\n");
	return 0;
}

int MFS_Write(int inum, char *buffer, int block)
{
	printf("[MFS_Write]\n");
	return 0;
}

int MFS_Read(int inum, char *buffer, int block)
{
	printf("[MFS_Read]\n");
	return 0;
}

int MFS_Creat(int pinum, int type, char *name)
{
	printf("[MFS_Creat]\n");
	return 0;
}

int MFS_Unlink(int pinum, char *name)
{
	printf("[MFS_Unlink]\n");
	return 0;
}

int MFS_Shutdown()
{
	printf("[MFS_Shutdown]\n");
	return 0;
}