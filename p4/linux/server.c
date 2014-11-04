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
**	HTTP connection struct
*/
/*
typedef struct HttpConnection 
{
	int ConnectionSocket;
}
HttpConnection;
*/

/*
**	Global variables: these will be shared between all threads and the main process
*/
//HttpConnection *HttpConnectionBuffers;
int ActiveConnection = 0;
int *HttpConnections;
int NumBuffers;
int NumThreads;
int PortNumber;
int ThreadCounter = 0;
pthread_t *ThreadPool;
pthread_mutex_t ConnectionBufferLock;


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
	
	printf("[HttpConnectionHandler-%d] thread started\n", ThisThreadIndex);

	// Main thread loop
	while(1)
	{
		printf("[HttpConnectionHandler-%d] requesting lock\n", ThisThreadIndex);
		if(pthread_mutex_lock(&ConnectionBufferLock) != 0)
		{
			printf("[HttpConnectionHandler-%d] unable to get lock\n", ThisThreadIndex);
		}
		printf("[HttpConnectionHandler-%d] thread unlocked!\n", ThisThreadIndex);
		
		printf("[HttpConnectionHandler-%d] calling request handle on fd=%d...\n", ThisThreadIndex, HttpConnections[ActiveConnection]);
		//requestHandle(ConnectionSocket);
		requestHandle(HttpConnections[ActiveConnection]);
		Close(HttpConnections[ActiveConnection]);
		
		// Increment ActiveConnection, or reset it to 0 if it has reached the last buffer
		ActiveConnection = (ActiveConnection < (NumBuffers - 1)) ? ActiveConnection + 1 : 0;
		
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
    
    // Set up locks
	pthread_mutex_init(&ConnectionBufferLock, NULL);
	if(pthread_mutex_lock(&ConnectionBufferLock) != 0)
	{
		printf("[server] unable to set the initial buffer lock\n");
	}

	// Verify + store command line arguments
    getargs(argc, argv);
	printf("[server] Running with NumBuffers=%d, NumThreads=%d, PortNumber=%d\n", NumBuffers, NumThreads, PortNumber);

	// Set up connection buffers
	//HttpConnectionBuffers = (HttpConnectionBuffers*) malloc (sizeof(HttpConnectionBuffers*) * NumBuffers);
	HttpConnections = (int *) malloc (sizeof(int*) * NumBuffers);

    // Set up the thread pool, and initialize all threads
    ThreadPool = (pthread_t*) malloc (sizeof(pthread_t*) * NumThreads);
    for(i = 0; i < NumThreads; i ++)
    {
   		//printf("[server] creating new thread\n");
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
    	printf("[server] clientlen=%d\n", clientlen);
		clientlen = sizeof(clientaddr);
		printf("[server] waiting for connection...\n");
		ConnectionSocket = Accept(HttpListenSocket, (SA *)&clientaddr, (socklen_t *) &clientlen);
	
		// 
		// CS537: In general, don't handle the request in the main thread.
		// Save the relevant info in a buffer and have one of the worker threads 
		// do the work.
		// requestHandle(ConnectionSocket);
		
		printf("[server] got a connection at fd=%d! saving connection data to buffer\n", ConnectionSocket);
		HttpConnections[ActiveConnection] = ConnectionSocket;
		printf("[server] releasing lock\n");
		if(pthread_mutex_unlock(&ConnectionBufferLock) != 0)
		{
			printf("[server] unable to release lock\n");
		}		

    }

}


    


 
