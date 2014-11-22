/* join should not handle child processes (forked) */
#include "types.h"
#include "user.h"

#undef NULL
#define NULL ((void*)0)

#define PGSIZE (4096)

int ppid;
int global = 1;

#define assert(x) if (x) {} else { \
   printf(1, "%s: %d ", __FILE__, __LINE__); \
   printf(1, "assert failed (%s)\n", # x); \
   printf(1, "TEST FAILED\n"); \
   kill(ppid); \
   exit(); \
}

int
main(int argc, char *argv[])
{
   ppid = getpid();

	printf(1, "[join2(%d)] about to fork\n", getpid());
   int fork_pid = fork();
   printf(1, "[join2(%d)] fork_pid=%d\n", getpid(), fork_pid);
   if(fork_pid == 0) {
     exit();
     
   }
   assert(fork_pid > 0);

	printf(1, "[join2(%d)] about to join\n", getpid());
   int join_pid = join();
   printf(1, "[join2(%d)] join_pid=%d\n", getpid(), join_pid);
   assert(join_pid == -1);

   printf(1, "TEST PASSED\n");
   exit();
}
