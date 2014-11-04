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

int NumBuffers;
int NumThreads;
int PortNumber;
pthread_t *ThreadPool;


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
**
*/

void HttpConnectionHandler()
{
	printf("[HttpConnectionHandler] called\n");
}


/*
**	Main server program
*/

int main(int argc, char *argv[])
{
    int HttpListenFD, connfd, clientlen;
    int i;
    
    struct sockaddr_in clientaddr;

	// Verify + store command line arguments
    getargs(argc, argv);
	printf("[server] Running with NumBuffers=%d, NumThreads=%d, PortNumber=%d\n", NumBuffers, NumThreads, PortNumber);

    // Set up the thread pool, and initialize all threads
    ThreadPool = (pthread_t*) malloc (sizeof(pthread_t*) * NumThreads);
    for(i = 0; i < NumThreads; i ++)
    {
   		printf("[server] creating new thread\n");
    	if(pthread_create(&ThreadPool[i], NULL, &HttpConnectionHandler, NULL) != 0)
    	{
    		perror("Failed to create HTTP connection handler thread.\n");
    	}
    }

	// Open the HTTP connection
    HttpListenFD = Open_listenfd(PortNumber);

    // Main server loop waiting for connections
    while (1) 
    {
    	printf("[server] clientlen=%d\n", clientlen);
		clientlen = sizeof(clientaddr);
		printf("[server] waiting for connection...\n");
		connfd = Accept(HttpListenFD, (SA *)&clientaddr, (socklen_t *) &clientlen);
	
		// 
		// CS537: In general, don't handle the request in the main thread.
		// Save the relevant info in a buffer and have one of the worker threads 
		// do the work.
		// 
		printf("[server] calling request handle...\n");
		requestHandle(connfd);
	
		Close(connfd);
    }

}


    


 
