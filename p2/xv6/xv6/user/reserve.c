#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{
	int ReservePercent;

	if(argc == 1)
	{
		printf(1, "Error: missing reserve percent!\nUsage: $ reserve <percent>\n");
		exit();
	}
	
	// Determine the amount of cpu time to reserve
	ReservePercent = atoi(argv[1]);
	if(ReservePercent == 0)
	{
		printf(1, "Error: reserve percent must be a valid interger!\nUsage: $ reserve <percent>\n");
		exit();
	}

	// Now try to reserve the time
	if(reserve(ReservePercent) != 0)
	{
		printf(1, "Error: failed to reserve cpu time\n");
		exit();
	}
	
	// Waste a bunch of time
	printf(1, "Reserved %d% of CPU, now just wasting some time...\n", ReservePercent);
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