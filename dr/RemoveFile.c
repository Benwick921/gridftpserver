#include "drutil.h"

int removefile(char* path)
{
	if(!remove(path)) return 0;
	return 1;
}
