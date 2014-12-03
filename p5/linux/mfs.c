#include <stdio.h>

#include "mfs.h"
#include "udp.h"

/*
**	Global variables
*/

struct sockaddr_in UDPSocket;

int MFS_Init(char *hostname, int port)
{
	// Declare variables
	int SocketDescriptor = UDP_Open(0);
	message InitRequest;	

	// Fill the UDPSocket object
	if(UDP_FillSockAddr(&UDPSocket, hostname, port) != 0)
	{
		printf("[MFS_Init] Error filling UDPSocket object\n");
		return -1;
	}
	
	// Send the INIT message
	strcpy(InitRequest.cmd, "INIT");
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
		perror("[MFS_Lookup] Error: UDPSocket not initialized\n");
		return -1;
	}
	
	// Send the LOOKUP message
	strcpy(LookupRequest.cmd, "LOOKUP");
	LookupRequest.inum = pinum;
	strcpy(LookupRequest.name, name);
	BytesSent = UDP_Write(SocketDescriptor, &UDPSocket, (char*)&LookupRequest, sizeof(message));
	
	// Wait for the response
	BytesReceived = UDP_Read(SocketDescriptor, &UDPSocket, (char*)&LookupResponse, sizeof(response));
	printf("[MFS_Lookup] Received response, BytesReceived=%d, LookupResponse.rc=%d\n", BytesReceived, LookupResponse.rc);
	
	// Return the lookup response code (-1 if failure, or inode # if success)
	return LookupResponse.rc;
}

int MFS_Stat(int inum, MFS_Stat_t *m)
{
	// Declare variables
	int BytesReceived;
	int BytesSent;
	int SocketDescriptor = UDP_Open(0);
	message StatRequest;
	response StatResponse;	

	// Verify that UDPSocket has been initialized
	if(UDPSocket.sin_port <= 0)
	{
		perror("[MFS_Stat] Error: UDPSocket not initialized\n");
		return -1;
	}
	
	// Send the STAT message
	strcpy(StatRequest.cmd, "STAT");
	StatRequest.inum = inum;
	BytesSent = UDP_Write(SocketDescriptor, &UDPSocket, (char*)&StatRequest, sizeof(message));
	
	// Wait for the response
	BytesReceived = UDP_Read(SocketDescriptor, &UDPSocket, (char*)&StatResponse, sizeof(response));
	m->type = StatResponse.stat.type;
	m->size = StatResponse.stat.size;
	printf("[MFS_Stat] Received response, BytesReceived=%d, StatResponse.rc=%d, m->size=%d\n", BytesReceived, StatResponse.rc, m->size);
	
	// Return the lookup response code (-1 if failure, 0 if success)
	return StatResponse.rc;
}

int MFS_Write(int inum, char *buffer, int block)
{
	// Declare variables
	int BytesReceived;
	int BytesSent;
	int SocketDescriptor = UDP_Open(0);
	message WriteRequest;
	response WriteResponse;	

	// Verify that UDPSocket has been initialized
	if(UDPSocket.sin_port <= 0)
	{
		perror("[MFS_Write] Error: UDPSocket not initialized\n");
		return -1;
	}
	
	// Send the WRITE message
	strcpy(WriteRequest.cmd, "WRITE");
	WriteRequest.inum = inum;
	strcpy(WriteRequest.block, buffer);
	WriteRequest.blocknum = block;
	BytesSent = UDP_Write(SocketDescriptor, &UDPSocket, (char*)&WriteRequest, sizeof(message));
	
	// Wait for the response
	BytesReceived = UDP_Read(SocketDescriptor, &UDPSocket, (char*)&WriteResponse, sizeof(response));
	printf("[MFS_Write] Received response, BytesReceived=%d, WriteResponse.rc=%d\n", BytesReceived, WriteResponse.rc);
	
	// Return the lookup response code (-1 if failure, 0 if success)
	return WriteResponse.rc;
}

int MFS_Read(int inum, char *buffer, int block)
{
	// Declare variables
	int BytesReceived;
	int BytesSent;
	int SocketDescriptor = UDP_Open(0);
	message ReadRequest;
	response ReadResponse;	

	// Verify that UDPSocket has been initialized
	if(UDPSocket.sin_port <= 0)
	{
		perror("[MFS_Read] Error: UDPSocket not initialized\n");
		return -1;
	}
	
	// Send the READ message
	strcpy(ReadRequest.cmd, "READ");
	ReadRequest.inum = inum;
	ReadRequest.blocknum = block;
	BytesSent = UDP_Write(SocketDescriptor, &UDPSocket, (char*)&ReadRequest, sizeof(message));
	
	// Wait for the response
	BytesReceived = UDP_Read(SocketDescriptor, &UDPSocket, (char*)&ReadResponse, sizeof(response));
	if(ReadResponse.rc > 0)
	{
		strcpy(buffer, ReadResponse.block);
	}
	printf("[MFS_Read] Received response, BytesReceived=%d, ReadResponse.rc=%d, buffer=%s\n", BytesReceived, ReadResponse.rc, buffer);
	
	// Return the lookup response code (-1 if failure, 0 if success)
	return ReadResponse.rc;

}

int MFS_Creat(int pinum, int type, char *name)
{
	// Declare variables
	int BytesReceived;
	int BytesSent;
	int SocketDescriptor = UDP_Open(0);
	message CreatRequest;
	response CreatResponse;	

	// Verify that UDPSocket has been initialized
	if(UDPSocket.sin_port <= 0)
	{
		perror("[MFS_Write] Error: UDPSocket not initialized\n");
		return -1;
	}
	
	// Send the CREAT message
	strcpy(CreatRequest.cmd, "CREAT");
	CreatRequest.inum = pinum;
	strcpy(CreatRequest.name, name);
	BytesSent = UDP_Write(SocketDescriptor, &UDPSocket, (char*)&CreatRequest, sizeof(message));
	
	// Wait for the response
	BytesReceived = UDP_Read(SocketDescriptor, &UDPSocket, (char*)&CreatResponse, sizeof(response));
	printf("[MFS_Write] Received response, BytesReceived=%d, WriteResponse.rc=%d\n", BytesReceived, CreatResponse.rc);
	
	// Return the lookup response code (-1 if failure, 0 if success)
	return CreatResponse.rc;
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