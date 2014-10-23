/*Test memory being freed in the middle of memory and then reallocated*/
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "mem.h"

int main() {

	//Request memory
	
	assert(Mem_Init(4096) == 0);
	void * ptr[6];
	
	//Allocate memory

	ptr[0] = Mem_Alloc(16);
	assert(ptr[0] != NULL);

	ptr[1] = Mem_Alloc(80);
	assert(ptr[1] != NULL);

	ptr[2] = Mem_Alloc(16);
	assert(ptr[2] != NULL);
	
	ptr[3] = Mem_Alloc(80);
	assert(ptr[3] != NULL);
	
	ptr[4] = Mem_Alloc(80);
	assert(ptr[4] != NULL);
	
	ptr[5] = Mem_Alloc(16);
	assert(ptr[5] != NULL);
	
	//Fill up the rest of the page
	while (Mem_Alloc(16) != NULL);
	
	//Free memory in the middle

	assert(Mem_Free(ptr[1]) == 0);
	ptr[1] = NULL;
	
	assert(Mem_Free(ptr[2]) == 0);
	ptr[2] = NULL;
	
	assert(Mem_Free(ptr[3]) == 0);
	ptr[3] = NULL;
	
	assert(Mem_Free(ptr[4]) == 0);
	ptr[4] = NULL;

	//Allocate newly freed memory

	ptr[1] = Mem_Alloc(256);
	assert(ptr[1] != NULL);
	
	printf("The test succeeded!\n");

	exit(0);
}
