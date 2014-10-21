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
	
	TestString1 = (char*)Mem_Alloc(16);
	TestString2 = (char*)Mem_Alloc(64); 
	TestString3 = (char*)Mem_Alloc(256);
	
	// Print out the first eight bytes pointed to by TestString
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
	
	// Display memory dump
	Mem_Dump();
	
	// All done!
	return 0;	
}