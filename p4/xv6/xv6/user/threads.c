#include "types.h"
#include "stat.h"
#include "user.h"

int x = 0;
//int stack[1024] __attribute__ ((aligned (4096)));

int main(int argc, char *argv[]) 
{
	printf(1, "Process table:\n");
	join();
	
	int test = 666;
	
	int* stack;
	stack = (int*)malloc(1024 * sizeof(int*));
	
    //int tid = fork();
    printf(1, "[threads] Calling clone, stack address=0x%x, test address=0x%x, x address=0x%x\n", stack, &test, &x);
  	int tid = clone(stack);
	
	printf(1, "\n\nProcess table after clone call:\n");
	join();
	
	printf(1, "[threads] tid=%d\n", tid);	
	
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