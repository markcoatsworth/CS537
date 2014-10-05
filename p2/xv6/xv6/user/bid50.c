#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{
	if(spot(50) != 0)
	{
		printf(1, "Failed to bid for cpu time\n");
	}
	
	// Waste a bunch of time
	printf(1, "Bid 50 for a spot process, now just wasting some time...\n");
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