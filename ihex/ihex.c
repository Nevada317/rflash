#include "ihex.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define MaxStringLength 4096

static bool _try_to_add_seg(datasegment_t** ptr_root, char* line) {
	(void)ptr_root;
	printf("Line: %s\n", line);
	return false;
}

bool IHEX_AppendHex(datasegment_t** ptr_root, char* filename) {
	FILE *fp;
	fp = fopen(filename , "r");
	if (!fp) {
		printf("Error opening file '%s'", filename);
		return false;
	}
	bool hasSegments;
	char str[MaxStringLength+1];
	char* lfptr;
	while (1) {
		// Clear string
		str[0] = '\0';
		// Try to read line
		if (!fgets(str, MaxStringLength+1, fp)) break;
		// Remove trailing newline
		if (lfptr = strchr(str, '\n')) *lfptr = '\0';
		// Parse line
		if (_try_to_add_seg(ptr_root, str)) hasSegments = true;
	}
	return hasSegments;
}
