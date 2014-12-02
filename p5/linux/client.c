#include <stdio.h>
#include <stdlib.h>

#include "mfs.h"
#include "udp.h"

int main(int argc, char *argv[])
{
	
    
    // Open the server on the specified port
    MFS_Init("127.0.0.1", 1666);
    
    return 0;
}