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
 *	Create a new AllocNode item, add it to the allocation list, leave the size of bytes requested after it
 *	Then adjust other nodes in the allocation list accordingly
 */
void* Mem_Alloc(int size)
{
	// Declare variables
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
	//printf("[Mem_Alloc] Found target node at location %zu [%p]\n", (long unsigned int)TargetNode, (void*)TargetNode);
	if(TargetNode->Size == AllocSize)
	{
		return NULL;
	}
	// If the target node is larger than the requested allocation size, we need to break it up
	else if(TargetNode->Size > AllocSize)
	{
		AllocNode *NewNode;
		// Bug: following line adds 256 + 256 instead of 16 + 16 (which it's supposed to)
		// So the location of NewNode is incorrect. Everything else seems okay though.
		NewNode = TargetNode + sizeof(AllocNode) + AllocSize;
		// printf("[Mem_Alloc] NewNode goes into %zu + %zu + %d = %zu\n", (long unsigned int)TargetNode, sizeof(AllocNode), AllocSize, (long unsigned int)NewNode);
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