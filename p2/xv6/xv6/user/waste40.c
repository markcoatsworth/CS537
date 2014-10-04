#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{
	if(reserve(40) != 0)
	{
		printf(1, "Failed to reserve cpu time\n");
	}
	
	// Waste a bunch of time
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