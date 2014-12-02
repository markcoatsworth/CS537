#include <stdio.h>

#include "mfs.h"
#include "udp.h"

/*
**	Global variables
*/

struct sockaddr_in UDPSocket;

int MFS_Init(char *hostname, int port)
{
	printf("[MFS_Init] hostname=%s, port=%d\n", hostname, port);

	// Declare variables
	int SocketDescriptor = UDP_Open(0);
	message InitRequest;	

	// Fill the UDPSocket object
	if(UDP_FillSockAddr(&UDPSocket, hostname, port) != 0)
	{
		printf("[MFS_Init] Error filling UDPSocket object\n");
	}
	
	// Send the initialize message
	InitRequest.type = 0;
	UDP_Write(SocketDescriptor, &UDPSocket, (char*)&InitRequest, sizeof(message));
	
	return 0;
}

int MFS_Lookup(int pinum, char *name)
{
	// Declare variables
	int BytesReceived;
	int BytesSent;
	int SocketDescriptor = UDP_Open(0);
	message LookupRequest;
	response LookupResponse;	

	// Verify that UDPSocket has been initialized
	if(UDPSocket.sin_port <= 0)
	{
		printf("[MFS_Lookup] Error: need to initialize UDPSocket before calling MFS_Lookup\n");
	}
	
	// Send the initialize message
	LookupRequest.type = 1;
	LookupRequest.inum = pinum;
	strcpy(LookupRequest.name, name);
	BytesSent = UDP_Write(SocketDescriptor, &UDPSocket, (char*)&LookupRequest, sizeof(message));
	
	// Wait for the response
	BytesReceived = UDP_Read(SocketDescriptor, &UDPSocket, (char*)&LookupResponse, sizeof(response));
	printf("[MFS_Lookup] Received response, BytesReceived=%d, InitResponse.rc=%d\n", BytesReceived, LookupResponse.rc);
	
	// Return the lookup response code (-1 if failure, or inode # if success)
	return LookupResponse.rc;
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