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
	int ValidLookup = MFS_Lookup(0, "test1.txt");
    printf("[client] MFS_Lookup returned %d for the valid lookup name\n", ValidLookup);
    
    // Now look up a bogus directory to test for error
	int BogusLookup = MFS_Lookup(0, "666.txt");
    printf("[client] MFS_Lookup returned %d for the bogus lookup name\n", BogusLookup);
    
    // Get some stats on the valid lookup
	MFS_Stat_t ValidStat;
    int ValidStatReturnCode = MFS_Stat(ValidLookup, &ValidStat);
    printf("[client] MFS_Stat returned %d for the valid stat lookup; ValidStat.size=%d, ValidStat.type=%d\n", ValidStatReturnCode, ValidStat.size, ValidStat.type);

	// Get some stats on a bogus inode value
	MFS_Stat_t BogusStat;
    int BogusStatReturnCode = MFS_Stat(666, &BogusStat);
    printf("[client] MFS_Stat returned %d for the bogus stat lookup; BogusStat.size=%d, BogusStat.type=%d\n", BogusStatReturnCode, BogusStat.size, BogusStat.type);

    
    return 0;
}