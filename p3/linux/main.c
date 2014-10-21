#include <stdio.h>
#include "mem.h"


int main()
{

	// Mem_Init is the first step in all programs that we write
	// Gets a huge chunk of memory for you
	// All subsequent functions will manage this chunk of memory
	
	// Main file is just a tester -- all the functionality should be in the library
	printf("Hello world\n");
	Mem_Init(16);
	Mem_Dump();
	return 0;
	
}