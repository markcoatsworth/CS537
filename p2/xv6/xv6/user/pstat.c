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
	int test = getpinfo(userpstat);
	printf(1, "getpinfo returned %d\n", test);
	/*
	if(getpinfo(userpstat) != 0)
	{
		printf(1, "Error: system call to getpinfo failed\n");
		return 1;
	}
	*/
	
	// Now display the info of all active processes
	int i;
	printf(1, "\tINUSE\t\tPID\t\tCHOSEN\t\tTIME\t\tCHARGE\n");
	for(i = 0; i < 64; i ++)
	{
		//if(userpstat->inuse[i] == 1)
		//{
			printf(1, "[%d]\t%d\t\t%d\t\t%d\t\t%d\t\t%d\n", i, userpstat->inuse[i], userpstat->pid[i], userpstat->chosen[i], userpstat->time[i], userpstat->charge[i]);
		//}
	}
	
	exit();

}