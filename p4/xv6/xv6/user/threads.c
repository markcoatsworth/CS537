#include "types.h"
#include "stat.h"
#include "user.h"

int stack[4096] __attribute__ ((aligned (4096)));
int x = 0;

int main(int argc, char *argv[]) 
{
	printf(1, "Process table:\n");
	join();
  
    //int tid = fork();
  	int tid = clone(stack);
	
	printf(1, "\n\nProcess table after clone call:\n");
	join();
	
  	if (tid < 0) 
  	{
    	printf(2, "error!\n");
  	} 
  	else if (tid == 0) 
  	{
    	// child
    	for(;;) 
    	{
      		x++;
	  		printf(1, "[threads.child(%d)] x = %d\n", getpid(), x);
      		sleep(100);
    	}
  	} 
  	else 
  	{
    	// parent
    	for(;;) 
    	{
      		printf(1, "[threads.parent(%d)] x = %d\n", getpid(), x);
      		sleep(100);
    	}
  	}

  	exit();
}