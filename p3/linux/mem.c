#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "mem.h"

typedef struct __AllocNode {
	int IsAllocated;
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
	HeadNode->IsAllocated = 0;
	HeadNode->Size = InitSize - sizeof(AllocNode);
	HeadNode->Next = NULL;	
	AllocListHead = HeadNode;
	
	// All done! Return
	printf("Got memory at %p\n", ptr);
	return 0;
}

/*
 *	Allocate the memory requested by the size parameter.
 *	Find a block of free memory (ideally one which is exactly the requested size) and return a pointer
 */
void* Mem_Alloc(int size)
{
	// Declare variables
	AllocNode* NewNode;
	AllocNode* TargetNode;
	AllocNode* ThisNode = AllocListHead;
	int AllocSize = size;
	
	// Make sure allocation size is a multiple of 8. Allocate extra memory if necessary.
	if((AllocSize % 8) != 0)
	{
		AllocSize += 8 - (AllocSize % 8);
	}
	
	// Traverse the allocation list, determine the best place for new memory to go
	while(ThisNode != NULL)
	{
		// If this node matches the allocation size exactly, use it, and break out of the loop
		if(ThisNode->IsAllocated == 0 && ThisNode->Size == AllocSize)
		{
			TargetNode = ThisNode;
			break;
		}
		// If this node is larger than the allocation size, temporarily select it, although we may still find an exact match
		else if(ThisNode->IsAllocated == 0 && ThisNode->Size > AllocSize)
		{
			TargetNode = ThisNode;
		}
		
		ThisNode = ThisNode->Next;
	}

	
	// Make sure there is still enough space for the space requested; if not, return NULL
	if(TargetNode->Size < (AllocSize + sizeof(AllocNode)))
	{
		return NULL;
	}
	
	// If we found an exact match, then set it to allocated and return it
	if(TargetNode->Size == AllocSize)
	{
		TargetNode->IsAllocated = 1;
	}
	// If we found a node that was bigger, then split it
	else
	{
		NewNode = TargetNode + sizeof(AllocNode)/16 + AllocSize/16;
		NewNode->IsAllocated = 0;
		NewNode->Size = TargetNode->Size - sizeof(AllocNode) - AllocSize;
		NewNode->Next = NULL;
		TargetNode->IsAllocated = 1;
		TargetNode->Size = AllocSize;
		TargetNode->Next = NewNode;
	}
	
	//printf("[Mem_Alloc] TargetNode @ %p, NewNode @ %p\n", (void*)ThisNode, (void*)NewNode);
	
	// All done! Return the memory location address of the allocated memory
	// (which is the location of the ThisNode pointer, plus the space reserved for the node structure)
	return (void *)(TargetNode + sizeof(AllocNode)/16);
}

int Mem_Free(void* ptr)
{
	// Declare variables
	AllocNode* ThisNode = AllocListHead;
	//int FreedNodeSize = 0;
	
	// First, check that ptr is valid
	if(ptr == NULL)
	{
		return -1;
	}
	
	// Determine which node structure has allocated the memory pointed to by ptr
	while(ThisNode != NULL)
	{
		if((ThisNode + sizeof(AllocNode)/16) == ptr)
		{
			break;
		}
		ThisNode = ThisNode->Next;
	}
	
	// If ThisNode->Next now points to null, then nothing in our list matched the parameter. Exit now.
	if(ThisNode == NULL)
	{
		return -1;
	}
	
	// TO deallocate ThisNode, simply set its IsAllocated value to 0
	ThisNode->IsAllocated = 0;
	
	
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
		printf("[Mem_Dump] Address: %zu [%p], Size: %d, IsAllocated: %d\n", (long unsigned int)ThisNode + sizeof(AllocNode), (void*)ThisNode + sizeof(AllocNode), ThisNode->Size, ThisNode->IsAllocated);
		ThisNode = ThisNode->Next;
	}
}