#include <stdio.h>

#include "mfs.h"
#include "udp.h"

/*
**	Global variables
*/

struct sockaddr_in UDPSocket;

/*
** 	int MFS_Init(char *hostname, int port): 
** 	MFS_Init() takes a host name and port number and uses those to find the server exporting the file system.
*/
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

/*
**	int MFS_Lookup(int pinum, char *name): 
**	MFS_Lookup() takes the parent inode number (which should be the inode number of a directory) and looks up the entry name in it. 
**	The inode number of name is returned. 
**	Success: return inode number of name; failure: return -1. 
**	Failure modes: invalid pinum, name does not exist in pinum.
*/
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
	
	// Verify that name is not too long
	if(strlen(name) > 60)
	{
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

/*
**	int MFS_Stat(int inum, MFS_Stat_t *m): 
**	MFS_Stat() returns some information about the file specified by inum. 
**	Upon success, return 0, otherwise -1. The exact info returned is defined by MFS_Stat_t. 
**	Failure modes: inum does not exist.
*/
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

/*
**	int MFS_Write(int inum, char *buffer, int block): 
**	MFS_Write() writes a block of size 4096 bytes at the block offset specified by block.
**	Returns 0 on success, -1 on failure. 
**	Failure modes: invalid inum, invalid block, not a regular file (because you can't write to directories).
*/
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

/*
**	int MFS_Read(int inum, char *buffer, int block): 
**	MFS_Read() reads a block specified by block into the buffer from file specified by inum. 
**	The routine should work for either a file or directory; directories should return data in the format specified by MFS_DirEnt_t. 
**	Success: 0, failure: -1. 
**	Failure modes: invalid inum, invalid block.
*/
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

/*
**	int MFS_Creat(int pinum, int type, char *name): 
**	MFS_Creat() makes a file ( type == MFS_REGULAR_FILE) or directory ( type == MFS_DIRECTORY) in the parent directory specified by pinum of name name. 
**	Returns 0 on success, -1 on failure. 
**	Failure modes: pinum does not exist, or name is too long. 
**	If name already exists, return success (think about why).
*/
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
	
	// Verify that name is not too long
	if(strlen(name) > 60)
	{
		return -1;
	}
	
	// Send the CREAT message
	strcpy(CreatRequest.cmd, "CREAT");
	CreatRequest.inum = pinum;
	CreatRequest.type = type;
	strcpy(CreatRequest.name, name);
	BytesSent = UDP_Write(SocketDescriptor, &UDPSocket, (char*)&CreatRequest, sizeof(message));
	
	// Wait for the response
	BytesReceived = UDP_Read(SocketDescriptor, &UDPSocket, (char*)&CreatResponse, sizeof(response));
	printf("[MFS_Creat] Received response, BytesReceived=%d, WriteResponse.rc=%d\n", BytesReceived, CreatResponse.rc);
	
	// Return the lookup response code (-1 if failure, 0 if success)
	if(CreatResponse.rc < 0)
	{
		return -1;
	}
	
	return 0;
}

/*
**	int MFS_Unlink(int pinum, char *name): 
**	MFS_Unlink() removes the file or directory name from the directory specified by pinum. 
**	0 on success, -1 on failure. 
**	Failure modes: pinum does not exist, directory is NOT empty. 
**	Note that the name not existing is NOT a failure by our definition (think about why this might be).
*/
int MFS_Unlink(int pinum, char *name)
{
	// Declare variables
	int BytesReceived;
	int BytesSent;
	int SocketDescriptor = UDP_Open(0);
	message UnlinkRequest;
	response UnlinkResponse;	

	// Verify that UDPSocket has been initialized
	if(UDPSocket.sin_port <= 0)
	{
		perror("[MFS_Write] Error: UDPSocket not initialized\n");
		return -1;
	}
	
	// Verify that name is not too long
	if(strlen(name) > 60)
	{
		return -1;
	}
	
	// Send the CREAT message
	strcpy(UnlinkRequest.cmd, "UNLINK");
	UnlinkRequest.inum = pinum;
	strcpy(UnlinkRequest.name, name);
	BytesSent = UDP_Write(SocketDescriptor, &UDPSocket, (char*)&UnlinkRequest, sizeof(message));
	
	// Wait for the response
	BytesReceived = UDP_Read(SocketDescriptor, &UDPSocket, (char*)&UnlinkResponse, sizeof(response));
	printf("[MFS_Unlink] Received response, BytesReceived=%d, UnlinkResponse.rc=%d\n", BytesReceived, UnlinkResponse.rc);
	
	// Return the lookup response code (-1 if failure, 0 if success)
	if(UnlinkResponse.rc < 0)
	{
		return -1;
	}
	
	return 0;
}

/*
**	int MFS_Shutdown(): 
**	MFS_Shutdown() just tells the server to force all of its data structures to disk and shutdown by calling exit(0). 
**	This interface will mostly be used for testing purposes.
*/
int MFS_Shutdown()
{
	// Declare variables
	int BytesSent;
	int SocketDescriptor = UDP_Open(0);
	message ShutdownRequest;
	
	// Verify that UDPSocket has been initialized
	if(UDPSocket.sin_port <= 0)
	{
		perror("[MFS_Write] Error: UDPSocket not initialized\n");
		return -1;
	}
	
	// Send the SHUTDOWN message
	strcpy(ShutdownRequest.cmd, "SHUTDOWN");
	BytesSent = UDP_Write(SocketDescriptor, &UDPSocket, (char*)&ShutdownRequest, sizeof(message));
	
	// SHUTDOWN does not send back a response. Return success.
	return 0;
}

/*
**	int MFS_Debug(): 
**	MFS_Debug() dumps the contents of the file system to screen.
**	This interface will mostly be used for testing purposes.
*/
int MFS_Debug()
{
	// Declare variables
	int BytesReceived;
	int BytesSent;
	int SocketDescriptor = UDP_Open(0);
	message DebugRequest;
	response DebugResponse;	

	// Verify that UDPSocket has been initialized
	if(UDPSocket.sin_port <= 0)
	{
		perror("[MFS_Debug] Error: UDPSocket not initialized\n");
		return -1;
	}
	
	// Send the DEBUG message
	strcpy(DebugRequest.cmd, "DEBUG");
	BytesSent = UDP_Write(SocketDescriptor, &UDPSocket, (char*)&DebugRequest, sizeof(message));
	
	// Wait for the response
	BytesReceived = UDP_Read(SocketDescriptor, &UDPSocket, (char*)&DebugResponse, sizeof(response));
	
	return 0;
}