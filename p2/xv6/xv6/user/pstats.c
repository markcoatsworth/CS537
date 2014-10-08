#include "pstat.h"
#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{
	// Declare variables
	//char buf[1];
	/*
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
	*/
	
	// Check if turning on or off
	if(strcmp(argv[1], "off") != 0)
	{
		pstats(1);
	}
	else
	{
		pstats(0);
	}
	
	/*
	// Display the header row
	for(i = 0; i < 64; i ++)
	{
		if(userpstat->inuse[i] == 1)
		{
			printf(1, "%d (%s),", userpstat->pid[i], userpstat->pname[i]);
		}
	}
	*/
	
	// All done!
	exit();

}