#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{
	int BidAmount;

	if(argc == 1)
	{
		printf(1, "Error: missing bid amount!\nUsage: $ spotbid <amount>\n");
		exit();
	}
	
	// Determine the amount of cpu time to reserve
	BidAmount = atoi(argv[1]);
	if(BidAmount == 0)
	{
		printf(1, "Error: bid amount must be a valid interger!\nUsage: $ spotbid <amount>\n");
		exit();
	}

	// Now try to bid on the process
	if(spot(BidAmount) != 0)
	{
		printf(1, "Error: failed to bid for cpu time\n");
		exit();
	}

	// Waste a bunch of time
	printf(1, "Bid %d for a spot process, now just wasting some time...\n", BidAmount);
	long i;
	for(i = 0; i < 10000000; i ++)
	{
		float b = 5.67;
		float x = 4.3;
		float p = b / x;
		p = p + 1;
	}
	
	exit();

}