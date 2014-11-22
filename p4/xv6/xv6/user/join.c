/* clone and join syscalls */
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

void worker(void *arg_ptr);

int
main(int argc, char *argv[])
{
   ppid = getpid();

   void *stack = malloc(PGSIZE*2);
   assert(stack != NULL);
   if((uint)stack % PGSIZE)
     stack = stack + (4096 - (uint)stack % PGSIZE);

   int arg = 42;
   printf(1, "[join(%d)] calling clone...\n", getpid());
   int clone_pid = clone(stack);
   if (clone_pid == 0) {
     worker(&arg);
   }
   assert(clone_pid > 0);
	printf(1, "[join(%d)] calling join...\n", getpid());
   int join_pid = join();
   printf(1, "[join(%d)] join_pid=%d, clone_pid=%d\n", getpid(), join_pid, clone_pid);
   assert(join_pid == clone_pid);
   assert(global == 2);

   printf(1, "TEST PASSED\n");
   exit();
}

void
worker(void *arg_ptr) {
	printf(1, "[worker(%d)] thread started\n", getpid());
   int arg = *(int*)arg_ptr;
   assert(arg == 42);
   assert(global == 1);
   global++;
   	printf(1, "[worker(%d)] thread exiting\n", getpid());
   exit();
}
