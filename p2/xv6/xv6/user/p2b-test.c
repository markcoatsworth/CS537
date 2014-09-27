#include "pstat.h"
#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{

	struct pstat *syspstat;


	printf(1, "Called p2b-test\n");
	
	printf(1, "Calling reserve: %d\n", reserve(1));
	
	syspstat = malloc(sizeof(struct pstat));
	syspstat->inuse[0] = 1;
	printf(1, "sizeof syspstat=%d\n", sizeof(syspstat));
	
	printf(1, "Before getpinfo, syspstat->inuse[0]=%d\n", syspstat->inuse[0]);
	int success = getpinfo(syspstat);
	
	printf(1, "After getpinfo, syspstat->inuse[0]=%d\n", syspstat->inuse[0]);

	printf(1, "success=%d\n", success);
	
	//syspstat->inuse[0] = 666;
	
	exit();

}