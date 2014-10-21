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
 *	Allocate the memory requested by the calling funciton
 */
void* Mem_Alloc(int size)
{
	// Declare variables
	AllocNode* TargetNode;
	//AllocNode* TrailingNode;
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
		if(ThisNode->Size == AllocSize)
		{
			TargetNode = ThisNode;
			break;
		}
		// If this node is larger than the allocation size, temporarily select it, although we may still find an exact match
		else if(ThisNode->Size > AllocSize)
		{
			TargetNode = ThisNode;
		}
		
		ThisNode = ThisNode->Next;
	}
	
	// If TargetNode is still NULL, then we did not find a free space big enough; return null
	if(TargetNode == NULL)
	{
		return NULL;
	}
	
	// If the target node is the exact same size as the requested allocation size
	printf("[Mem_Alloc] Found target node at location %p, sizeof(AllocNode)=%zu\n", (void*)TargetNode, sizeof(AllocNode));
	if(TargetNode->Size == AllocSize)
	{
		return NULL;
	}
	// If the target node is larger than the requested allocation size, we need to break it up
	else if(TargetNode->Size > AllocSize)
	{
		AllocNode *NewNode;
		NewNode = TargetNode + sizeof(AllocNode) + AllocSize;
		NewNode->Size = TargetNode->Size - sizeof(AllocNode) - AllocSize;
		NewNode->Next = NULL;
		TargetNode->Size = AllocSize;
		TargetNode->Next = NewNode;
		return (void *)TargetNode;
	}
	
	return NULL;
	
}

int Mem_Free(void* ptr)
{
	return 0;
}

int Mem_Available()
{
	return 0;
}

/*
 *	Display the contents of the allocation list to screen. For debugging purposes only.
 */
void Mem_Dump()
{
	AllocNode* ThisNode = AllocListHead;
	while(ThisNode != NULL)
	{
		printf("Address: %p, Size: %d\n", (void*)ThisNode + sizeof(AllocNode), ThisNode->Size);
		ThisNode = ThisNode->Next;
	}
}