#include <stdio.h>
#include <stdlib.h>

#include "mfs.h"
#include "udp.h"

int main(int argc, char *argv[])
{
    // Open the server on the specified port
    int init = MFS_Init("127.0.0.1", 1666);
    printf("[client] MFS_Init returned %d\n", init);
    
    // Look up the name of the root directory
	char *LookupName;
	LookupName = (char*) malloc (100 * sizeof(char));
    int lookup = MFS_Lookup(0, LookupName);
    printf("[client] MFS_Lookup returned %d, LookupName=%s\n", lookup, LookupName);
    
    return 0;
}