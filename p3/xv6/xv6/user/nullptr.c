#include "types.h"
#include "stat.h"
#include "user.h"


int main()
{
		int* NullPointer;
		NullPointer = 0;
		
		printf(1, "Dereferencing a null pointer...\n");
		printf(1, "NullPointer=%d", *NullPointer);
		
		exit();
		
}