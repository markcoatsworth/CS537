#include <stdio.h>
#include "mem.h"


int main()
{
	// Initialize memory to 8 Kb
	Mem_Init(8096);
	
	// Display memory dump
	Mem_Dump();
	
	// Now allocate several dummy objects
	char *TestString1;
	char *TestString2;
	char *TestString3;
	char *TestString4;
	
	TestString1 = (char*)Mem_Alloc(16);
	printf("TestString1 lives at %p, memory still available: %d\n", (void*)TestString1, Mem_Available());
	TestString2 = (char*)Mem_Alloc(64); 
	printf("TestString2 lives at %p, memory still available: %d\n", (void*)TestString2, Mem_Available());
	TestString3 = (char*)Mem_Alloc(256);
	printf("TestString3 lives at %p, memory still available: %d\n", (void*)TestString3, Mem_Available());
	TestString4 = (char*)Mem_Alloc(7400);
	if(TestString4 == NULL)
	{
		printf("Could not allocate TestString4, not enough memory\n");
	}
	
	printf("Trying to free TestString2...\n");
	Mem_Free(TestString2);
	
	
	// Print out the first eight bytes pointed to by TestString
	/*
	printf("TestString1: ");
	int i;
	for(i = 0; i < 16; i ++)
	{
		printf("%d\t", TestString1[i]);
	}
	printf("\n");
	
	printf("TestString2: ");
	for(i = 0; i < 16; i ++)
	{
		printf("%d\t", TestString2[i]);
	}
	printf("\n");
	
	printf("TestString3: ");
	for(i = 0; i < 16; i ++)
	{
		printf("%d\t", TestString3[i]);
	}
	printf("\n");
	*/
	
	// Display memory dump
	Mem_Dump();
	
	// All done!
	return 0;	
}