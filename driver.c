// Shane DeSilva

#include <stdio.h>
#include "compiler.h"

int main(int argc, char **argv)
{
	printf("Source:\n");
	print_source(argv[1]);
	printf("\nParser:\n");
	parse(argv[1]);
	vm();
}
