#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "mem.h"

typedef struct __AllocNode {
	int Size;
	struct __AllocNode* Next;
} AllocNode;

AllocNode* AllocListHead;

/*
 *	Initialize the memory list
 */
int Mem_Init(int size)
{
	// Declare variables
	int InitSize = size;
	int SystemPageSize = getpagesize();
	
	// Verify that size is non-negative, and that we have not already called Mem_Init
	if(size <= 0 || AllocListHead != NULL) 
	{
		return -1;
	}

	// Ensure InitSize to be a multiple of page size. Increase InitSize if necessary.
	if((InitSize % SystemPageSize) != 0) 
	{
		InitSize += SystemPageSize - (InitSize % SystemPageSize);
	}
	
	// Request memory from the OS for the head node. Code taken from Vijay's class demo
	int fd = open("/dev/zero", O_RDWR);
	void *ptr = mmap(NULL, InitSize, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	if (ptr == MAP_FAILED)
	{
		perror("mmap");
		exit(1);
	}
	close(fd);
	
	// Now, point the global allocation head list pointer to the region we just allocated
	AllocNode* HeadNode = (AllocNode*) ptr;
	HeadNode->Next = NULL;
	HeadNode->Size = InitSize - sizeof(AllocNode);
	AllocListHead = HeadNode;
	
	// All done! Return
	printf("Got memory at %p\n", ptr);
	return 0;
}

/*
 *	Allocate the memory requested by the size parameter.
 *	Set the last pointer in the allocation list to the size of bytes requested
 *	Then adjust other nodes in the allocation list accordingly
 */
void* Mem_Alloc(int size)
{
	// Declare variables
	AllocNode* NewNode;
	AllocNode* ThisNode = AllocListHead;
	int AllocSize = size;
	
	// Make sure allocation size is a multiple of 8. Allocate extra memory if necessary.
	if((AllocSize % 8) != 0)
	{
		AllocSize += 8 - (AllocSize % 8);
	}
	
	// Determine the last node in the list (with Next pointing to NULL)
	// This is the node which references the free space in our memory
	while(ThisNode->Next != NULL)
	{
		ThisNode = ThisNode->Next;
	}
	
	// Make sure there is still enough space for the space requested; if not, return NULL
	if(ThisNode->Size < (AllocSize + sizeof(AllocNode)))
	{
		return NULL;
	}
	
	// Create a new data structure and store it in the free memory space	
	// Bug: following line adds 256 + 256 instead of 16 + 16 (which it's supposed to). So the location of NewNode is incorrect. Everything else seems okay though.
	NewNode = ThisNode + sizeof(AllocNode)/16 + AllocSize/16;
	NewNode->Size = ThisNode->Size - sizeof(AllocNode) - AllocSize;
	NewNode->Next = NULL;
	ThisNode->Size = AllocSize;
	ThisNode->Next = NewNode;

	//printf("[Mem_Alloc] ThisNode @ %p, NewNode @ %p\n", (void*)ThisNode, (void*)NewNode);
	
	// All done! Return the address of the allocated memory
	// (which is the location of the new node pointer, plus the space reserved for the node structure)
	return (void *)(ThisNode + sizeof(AllocNode)/16);
}

int Mem_Free(void* ptr)
{
	return 0;
}

/*
 *	Return number of bytes that can be used by future calls to Mem_Alloc
 *	Since we assume that only the last node in the AllocList (with Next pointing to NULL) contains
 *	memory that can still be allocated, return the Size value of this node.
 */
int Mem_Available()
{
	// Set the ThisNode pointer to the last node in the list
	AllocNode* ThisNode = AllocListHead;
	while(ThisNode->Next != NULL)
	{
		ThisNode = ThisNode->Next;
	}
	
	// Now that ThisNode is pointing at the last node in the list, return its size
	return ThisNode->Size;
}

/*
 *	Display the contents of the allocation list to screen. For debugging purposes only.
 */
void Mem_Dump()
{
	AllocNode* ThisNode = AllocListHead;
	//printf("[Mem_Dump] sizeof(int)=%zu\n", sizeof(int));
	//printf("[Mem_Dump] sizeof(AllocNode)=%zu\n", sizeof(AllocNode));
	//printf("[Mem_Dump] sizeof(*AllocNode)=%zu\n", sizeof(AllocNode*));
	while(ThisNode != NULL)
	{
		//printf("Address: %p, Size: %d\n", (void*)ThisNode + sizeof(AllocNode), ThisNode->Size);
		printf("[Mem_Dump] Address: %zu [%p], Size: %d\n", (long unsigned int)ThisNode + sizeof(AllocNode), (void*)ThisNode + sizeof(AllocNode), ThisNode->Size);
		ThisNode = ThisNode->Next;
	}
}