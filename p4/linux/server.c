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
pthread_cond_t BufferEmpty;
pthread_cond_t BufferFill;
pthread_mutex_t MutexLock;

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
void* HttpConnectionHandler()
{
	// Thread variables
	int ThisConnection;
	int ThisThreadIndex = ThreadCounter++; // only used for debug
	
	// Main thread loop
	while(1)
	{
		//printf("[HttpConnectionHandler-%d] Attempting to grab lock, MutexLock=%d...\n", ThisThreadIndex, MutexLock);
		// Get the lock
		if(pthread_mutex_lock(&MutexLock) != 0)
		{
			printf("[HttpConnectionHandler-%d] Unable to grab lock!\n", ThisThreadIndex);
		}
		
		// Release the lock and put the thread to sleep
		while(NumActiveConnections == 0)
		{
			if(pthread_cond_wait(&BufferFill, &MutexLock) != 0)
			{
				printf("[HttpConnectionHandler-%d] Error waiting for the fill signal!\n", ThisThreadIndex);
			}
		}
		
		// Thread is awake now!
		ThisConnection = HttpConnections[ActiveConnection];
		
		// Set the active buffer connection, and adjust the condition variable
		ActiveConnection = (ActiveConnection < (NumBuffers - 1)) ? ActiveConnection + 1 : 0;
		NumActiveConnections--;
				
		/*** Note: moving the requestHandle call outside the mutex results in tests 3, 4 passing, but many others failing ***/
		pthread_mutex_unlock(&MutexLock);
		
		// Now, the thread is awake and mutex is locked; unlock it and handle the request
		requestHandle(ThisConnection);
		Close(ThisConnection);
		
		if(pthread_cond_signal(&BufferEmpty) != 0)
		{
			printf("[HttpConnectionHandler-%d] Error sending the empty signal!\n", ThisThreadIndex);
		}
		
		
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
	
	// Initialize mutex variables
	pthread_mutex_init(&MutexLock, NULL);
	pthread_cond_init(&BufferEmpty, NULL);
	pthread_cond_init(&BufferFill, NULL);
	
	// Set up connection buffers
	HttpConnections = (int*) malloc (sizeof(int*) * NumBuffers);

    // Set up the thread pool, and initialize all threads
    ThreadPool = (pthread_t*) malloc (sizeof(pthread_t*) * NumThreads);
    for(i = 0; i < NumThreads; i ++)
    {
    	//printf("[server] Setting up thread %d\n", i);
   		if(pthread_create(&ThreadPool[i], NULL, &HttpConnectionHandler, NULL) != 0)
    	{
    		perror("Failed to create HTTP connection handler thread.\n");
    	}
    }

	// Open the HTTP connection
    HttpListenSocket = Open_listenfd(PortNumber);
    clientlen = sizeof(clientaddr);	  	
    

    // Main server loop waiting for connections
    while (1) 
    {
    	// Wait for a client connection
		ConnectionSocket = Accept(HttpListenSocket, (SA *)&clientaddr, (socklen_t *) &clientlen);
		
		// Once a connection has been made, acquire lock
		pthread_mutex_lock(&MutexLock);
   		
   		// If producer has already filled all buffers, wait
    	while(NumActiveConnections == NumBuffers)
    	{
    		if(pthread_cond_wait(&BufferEmpty, &MutexLock) != 0)
    		{
    			printf("[server] Error waiting for the empty signal!\n");
    		}
    	}
		
		// Add to the connections buffer and handle the locks
		HttpConnections[ActiveConnection] = ConnectionSocket;
		NumActiveConnections++;
		
		// All done this iteration, unlock the mutex
		pthread_mutex_unlock(&MutexLock);

		// Send the fill signal
		if(pthread_cond_signal(&BufferFill) != 0)
		{
			printf("[server] Error sending the fill signal!\n");
		}
		
		
    }

}