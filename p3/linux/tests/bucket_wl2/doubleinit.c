/* call init twice */
#include <assert.h>
#include <stdlib.h>
#include "mem.h"

int main() {
   assert(Mem_Init(4096) == 0);
   assert(Mem_Init(4096) == -1);
   exit(0);
}
