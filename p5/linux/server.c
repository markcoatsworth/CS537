#include <stdio.h>
#include <stdlib.h>

#include "udp.h"


int main(int argc, char *argv[])
{
	printf("[server]\n");
	
	char *hostname;
	hostname = (char*)malloc(10 * sizeof(char*));
	hostname = "testing";
	
	MFS_Init(hostname, 666);
	
	return 0;
}