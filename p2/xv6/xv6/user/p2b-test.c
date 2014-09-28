#include "pstat.h"
#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{
	int test;

	printf(1, "Calling reserve...\n");

	test = reserve(1);
	printf(1, "Reserve returned %d\n", test);

	
	
	
	exit();

}