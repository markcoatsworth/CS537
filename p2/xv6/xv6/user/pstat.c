#include "pstat.h"
#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{
	struct pstat *userpstat;

	// Allocate memory for the pstat table
	userpstat = malloc(sizeof(struct pstat));

	// Fire a system call to populate the pstat table
	if(getpinfo(userpstat) != 0)
	{
		printf(1, "Error: system call to getpinfo failed\n");
		return 1;
	}
	
	// Now display the info of all active processes
	int i;
	printf(1, "PID\tCHOSEN\tTIME\tCHARGE\n");
	for(i = 0; i < 20; i ++)
	{
		if(userpstat->inuse[i] == 1)
		{
			printf(1, "%d\t%d\t%d\t%d\n", 
				userpstat->pid[i],
				userpstat->chosen[i], 
				userpstat->time[i], 
				userpstat->charge[i]
			);
		}
	}
	
	exit();

}