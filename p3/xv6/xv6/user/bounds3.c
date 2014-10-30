/* syscall argument checks (stack boundaries) */
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#undef NULL
#define NULL ((void*)0)

#define assert(x) if (x) {} else { \
   printf(1, "%s: %d ", __FILE__, __LINE__); \
   printf(1, "assert failed (%s)\n", # x); \
   printf(1, "TEST FAILED\n"); \
   exit(); \
}

void foo() {
  int local[100];
  if((uint)&local >= 150*4096) foo();
}

int
main(int argc, char *argv[])
{
  char *arg;

  // ensure stack is actually high...
  assert((uint) &arg > 639*1024);

  int fd = open("tmp", O_WRONLY|O_CREATE);
  assert(fd != -1);

  printf(1, "grow the stack a bit\n");
  foo();
  uint STACK = 150*4096;
  uint USERTOP = 160*4096;

  printf(1, "below stack\n");
  arg = (char*) STACK - 1;
  assert(write(fd, arg, 1) == -1);

  printf(1, "spanning stack bottom\n");
  assert(write(fd, arg, 2) == -1);

  printf(1, "at stack\n");
  arg = (char*) STACK;
  assert(write(fd, arg, 1) != -1);

  printf(1, "within stack\n");
  arg = (char*) (STACK + 8192);
  assert(write(fd, arg, 40) != -1);

  printf(1, "at stack top\n");
  arg = (char*) USERTOP-1;
  assert(write(fd, arg, 1) != -1);

  printf(1, "spanning stack top\n");
  assert(write(fd, arg, 2) == -1);

  printf(1, "above stack top\n");
  arg = (char*) USERTOP;
  assert(write(fd, arg, 1) == -1);

  printf(1, "TEST PASSED\n");
  exit();
}
