// Do not modify this file. It will be replaced by the grading scripts
// when checking your project.

#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{
	int *p = 0;
	printf(1, "%s", "** Placeholder program for grading scripts **\n");
	printf(1, "%d\n", *p);
	// This produces the following;
	// ** Placeholder program for grading scripts **
	// -2082109099
	// We want to modify it so that we actually get a segfault when we access a bad address.
	// 
	exit();
}
