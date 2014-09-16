#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{
  	int addval;
  	
  	if(argc < 2)
  	{
    	printf(2, "Usage: addnum <value>\n");
    	exit();
  	}
  	else
  	{
  		addval = atoi(argv[1]);
  	}

  	printf(1, "Addnum test: %d\n", addnum(addval));
  	exit();
}
