#include "pstat.h"
#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{
	// Declare variables
	char buf[1];
	int i;
	struct pstat *userpstat;

	// Allocate memory for the pstat table
	userpstat = malloc(sizeof(struct pstat));
	
	// Fire a system call to populate the pstat table
	if(getpinfo(userpstat) != 0)
	{
		printf(1, "Error: system call to getpinfo failed\n");
		return 1;
	}

	// Display user text
	printf(1, "Displaying system process statistics. Press Enter key to exit.\n");

	// Turn on proces statistics
	pstats(1);
	
	// Display the header row
	for(i = 0; i < 64; i ++)
	{
		if(userpstat->inuse[i] == 1)
		{
			printf(1, "%d (%s),", userpstat->pid[i], userpstat->pname[i]);
		}
	}
	
	// Wait for user to press the enter key. Meanwhile, statistics are coming up on the screen.
	read(0, buf, 1);
	
	// Now turn pstats off
	pstats(0);
		
	// All done!
	exit();

}