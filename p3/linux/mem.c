#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "mem.h"

typedef struct __ListNode {
	int Value;
	struct __ListNode *Next;
} ListNode;

ListNode* AddressListHead;

int Mem_Init(int size)
{
	printf("Calling Mem_Init\n");
	int fd = open("/dev/zero", O_RDWR);
	
	int SystemPageSize;
	SystemPageSize = getpagesize();
	printf("System page size is %d\n", SystemPageSize);
	
	void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	if (ptr == MAP_FAILED)
	{
		perror("mmap");
		exit(1);
	}
	
	printf("Got memory at %p\n", ptr);
	
	close(fd);
	return 0;
}

void* Mem_Alloc(int size)
{
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

void Mem_Dump()
{
	
}