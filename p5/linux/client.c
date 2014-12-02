#include <stdio.h>
#include <stdlib.h>

#include "mfs.h"
#include "udp.h"

int main(int argc, char *argv[])
{
    // Open the server on the specified port
    int init = MFS_Init("127.0.0.1", 1666);
    printf("[client] MFS_Init returned %d\n", init);
    
    // Look up the name of a file under the root directory
	int lookup = MFS_Lookup(0, "test1.txt");
    printf("[client] MFS_Lookup returned %d for the valid lookup name\n", lookup);
    
    // Now look up a bogus directory to test for error
	lookup = MFS_Lookup(0, "666.txt");
    printf("[client] MFS_Lookup returned %d for the bogus lookup name\n", lookup);

    
    return 0;
}