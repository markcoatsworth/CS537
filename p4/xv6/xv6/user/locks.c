/* test lock correctness */
#include "types.h"
#include "user.h"

#undef NULL
#define NULL ((void*)0)

#define PGSIZE (4096)

int ppid;
int global = 0;
int locks;
int num_threads = 30;
int loops = 1000;


#define assert(x) if (x) {} else { \
   printf(1, "%s: %d ", __FILE__, __LINE__); \
   printf(1, "assert failed (%s)\n", # x); \
   printf(1, "TEST FAILED\n"); \
   kill(ppid); \
   exit(); \
}

void worker(void *arg_ptr);

int
main(int argc, char *argv[])
{
   ppid = getpid();

   int i;
   for (i = 0; i < num_threads; i++) {
   		printf(1, "[locks] creating thread %d\n", i);
      int thread_pid = thread_create(worker, 0);
      printf(1, "[locks] thread_pid=%d\n", thread_pid);
      assert(thread_pid > 0);
   }

   for (i = 0; i < num_threads; i++) {
   		printf(1, "[locks] joining thread %d\n", i);   
      int join_pid = join();
      printf(1, "[locks] join_pid=%d\n", join_pid);
      assert(join_pid > 0);
   }

   assert(global == num_threads * loops);

   printf(1, "TEST PASSED\n");
   exit();
}

void
worker(void *arg_ptr) {
	printf(1, "[worker(%d)] starting...\n", getpid());
   int i, j, tmp;
   for (i = 0; i < loops; i++) {
	//printf(1, "[worker(%d)] requesting lock\n", getpid());
      lock(&locks);
      tmp = global;
      for(j = 0; j < 50; j++); // take some time
      global = tmp + 1;
	//printf(1, "[worker(%d)] releasing lock\n", getpid());
      unlock(&locks);
   }
	printf(1, "[worker(%d)] ending...\n", getpid());   
   return;
}

