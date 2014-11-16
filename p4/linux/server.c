#include "cs537.h"
#include "request.h"

// 
// server.c: A very, very simple web server
//
// To run:
//  server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//


/*
**	Global variables: these will be shared between all threads and the main process
*/
int ActiveConnection = 0;
int *HttpConnections;
int NumActiveConnections = 0;
int NumBuffers;
int NumThreads;
int PortNumber;
int ThreadCounter = 0;
pthread_t *ThreadPool;
pthread_cond_t CondBufferLock;
pthread_mutex_t MutexBufferLock;


/*
**	Get command line arguments and save them to relevant pointers. Exit if any problems.
*/
void getargs(int argc, char *argv[])
{
    if (argc != 4) 
    {
		fprintf(stderr, "Usage: %s <portnum> <threads> <buffers>\n", argv[0]);
		exit(1);
    }
    PortNumber = atoi(argv[1]);
    NumThreads = atoi(argv[2]);
    NumBuffers = atoi(argv[3]);
}


/*
**	Thread function to handle HTTP connections
*/
void HttpConnectionHandler()
{
	// Set the thread index. This is only used for debug output.
	int ThisThreadIndex = ThreadCounter++;
	
	// Main thread loop
	while(1)
	{
		// Get the lock
		pthread_mutex_lock(&MutexBufferLock);
		
		// Release the lock and put the thread to sleep
		while(NumActiveConnections == 0)
		{
			//printf("[HttpConnectionHandler-%d] no active connections, throw wait\n", ThisThreadIndex);
			pthread_cond_wait(&CondBufferLock, &MutexBufferLock);
		}
		
		// Now, assuming the mutex is locked, unlock it and handle the request
		pthread_mutex_unlock(&MutexBufferLock);
		//printf("[HttpConnectionHandler-%d] calling request handle on fd=%d...\n", ThisThreadIndex, HttpConnections[ActiveConnection]);
		requestHandle(HttpConnections[ActiveConnection]);
		Close(HttpConnections[ActiveConnection]);
		
		// Set the active buffer connection, and adjust the condition variable
		ActiveConnection = (ActiveConnection < (NumBuffers - 1)) ? ActiveConnection + 1 : 0;
		NumActiveConnections--;
	}
}


/*
**	Main server program
*/
int main(int argc, char *argv[])
{
	// Define variables
    int HttpListenSocket, ConnectionSocket, clientlen;
    int i;    
    struct sockaddr_in clientaddr;
    
  	// Verify + store command line arguments
    getargs(argc, argv);
	printf("[server] Running with NumBuffers=%d, NumThreads=%d, PortNumber=%d\n", NumBuffers, NumThreads, PortNumber);

	// Set up connection buffers
	HttpConnections = (int *) malloc (sizeof(int*) * NumBuffers);

    // Set up the thread pool, and initialize all threads
    ThreadPool = (pthread_t*) malloc (sizeof(pthread_t*) * NumThreads);
    for(i = 0; i < NumThreads; i ++)
    {
   		if(pthread_create(&ThreadPool[i], NULL, &HttpConnectionHandler, NULL) != 0)
    	{
    		perror("Failed to create HTTP connection handler thread.\n");
    	}
    }

	// Open the HTTP connection
    HttpListenSocket = Open_listenfd(PortNumber);

    // Main server loop waiting for connections
    while (1) 
    {
    	// Wait for a client connection
    	clientlen = sizeof(clientaddr);
		ConnectionSocket = Accept(HttpListenSocket, (SA *)&clientaddr, (socklen_t *) &clientlen);
	
		// Once a connection comes in, add to the connections buffer and handle the locks
		HttpConnections[ActiveConnection] = ConnectionSocket;
		//printf("[server] releasing lock\n");
		pthread_mutex_lock(&MutexBufferLock);
		NumActiveConnections++;
		pthread_cond_signal(&CondBufferLock);
		pthread_mutex_unlock(&MutexBufferLock);
    }

}