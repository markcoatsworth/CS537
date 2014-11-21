#include "types.h"
#include "stat.h"
#include "user.h"

void nullfn(void* arg)
{
	while(1)
	{
		printf(1, "This is a dummy function! You should not be seeing this!!\n");
		sleep(100);
	}
}

void consumer(void* arg)
{
	while(1)
	{
		printf(1, "[consumer(%d)] thread running!\n", getpid());
		sleep(100);
	}
}

int main(int argc, char *argv[]) 
{
	printf(1, "[threads2(%d)] attempting to start the consumer thread, &consumer=0x%x\n", getpid(), &consumer);
	
	thread_create(&consumer, NULL);
	
	while(1)
	{
		printf(1, "[threads2(%d)] parent process running\n", getpid());
		sleep(100);
	}

  	exit();
}
