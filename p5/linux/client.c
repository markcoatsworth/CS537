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

	// Do a valid write
	int ValidWrite = MFS_Write(3, "This is a bunch of dummy text", 0);
	printf("[client] MFS_Write returned %d for the valid write request\n", ValidWrite);
	
	// Do a valid read (of the data we just wrote)
	char *ValidReadBuffer;
	ValidReadBuffer = (char*)malloc(MFS_BLOCK_SIZE * sizeof(char));
	int ValidRead = MFS_Read(3, ValidReadBuffer, 0);
	printf("[client] MFS_Read returned %d for the valid read, ReadBuffer=%s\n", ValidRead, ValidReadBuffer);
    
    // Now try a bogus read to an inode that doesn't exist
    char *BogusReadBuffer;
	BogusReadBuffer = (char*)malloc(MFS_BLOCK_SIZE * sizeof(char));
    int BogusRead = MFS_Read(3, BogusReadBuffer, 10);
    printf("[client] MFS_Read returned %d for the bogus read, ReadBuffer=%s\n", BogusRead, BogusReadBuffer);
    
	// Do a valid creat
	int ValidCreat = MFS_Creat(0, MFS_REGULAR_FILE, "creatfile.txt");    
	printf("[client] MFS_Creat returned %d for the valid creat\n", ValidCreat);
    
    return 0;
}