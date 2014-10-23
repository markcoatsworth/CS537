/*Test memory being freed above and below a current allocation and then reallocate that space*/
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "mem.h"

int main() {

	//Request memory
	
	assert(Mem_Init(4096) == 0);
	void * ptr[4];
	
	//Allocate memory

	ptr[0] = Mem_Alloc(256);
	assert(ptr[0] != NULL);

	ptr[1] = Mem_Alloc(80);
	assert(ptr[1] != NULL);

	ptr[2] = Mem_Alloc(256);
	assert(ptr[2] != NULL);
	
	//Fill up the rest of the page
	while (Mem_Alloc(16) != NULL);
	
	//Free memory above ptr[1]

	assert(Mem_Free(ptr[0]) == 0);
	ptr[0] = NULL;
	
	//Free memory below ptr[1]

	assert(Mem_Free(ptr[2]) == 0);
	ptr[2] = NULL;

	//Allocate newly freed memory

	ptr[0] = Mem_Alloc(256);
	assert(ptr[0] != NULL);
	
	ptr[2] = Mem_Alloc(80);
	assert(ptr[2] != NULL);

	ptr[3] = Mem_Alloc(256);
	assert(ptr[3] == NULL);

	printf("The test succeeded!\n");

	exit(0);
}
