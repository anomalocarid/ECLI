#include <stdio.h>
#include <stdlib.h>

#include "ecli.h"

int
main(int argc, char** argv)
{
	char* ver;
	char* fname;
	
	if(argc <= 2) {
		exit(EXIT_SUCCESS);
	}
	
	ver = argv[1];
	fname = argv[2];
	
	FILE* f = fopen(fname, "rb");
	if(f == NULL) {
		fprintf(stderr, "Failed to open file %s\n", fname);
		exit(EXIT_FAILURE);
	}
	
	th10_header_t header;
	
	fseek(f, 0, SEEK_SET);
	size_t amt = fread(&header, sizeof(th10_header_t), 1, f);
	if(amt != 1) {
		fprintf(stderr, "Failed to read ECL header.\n");
		exit(EXIT_FAILURE);
	}
	
	if((*(uint32_t*)&header.magic) != (*(uint32_t*)"SCPT")) {
		fprintf(stderr, "Invalid magic number: .\n");
		exit(EXIT_FAILURE);
	}
	
	printf("Include length: %d, include offset: %d, Sub count: %d\n",
	       header.include_length, header.include_offset, header.sub_count);
	
	fclose(f);
	return EXIT_SUCCESS;
}
