#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "translator.h"

#ifndef INTPTR_MAX
#include <inttypes.h>
#endif

int main(int argc, char *argv[])
{
	char *in, *out;
	intptr_t ptr;
	
	while (--argc) {
		if (strcmp(*(argv+argc), "-o") == 0)
			out = *(argv+argc+1);
		in = *(argv+argc);
	}
	printf("%s %s\n", in, out);
	
}
