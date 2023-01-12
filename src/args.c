#include "args.h"
#include <stdlib.h>
#include <string.h>

static void SetAppName(char** AppName, char* value) {
	if (!AppName) return;
	char* chrp = *AppName;

	if (*AppName) {
		free(*AppName);
		*AppName = 0;
	}
	if (value && strlen(value)) {
		*AppName = strdup(value);
	}
}

void ARGS_ParseArgs(char *argv[], char** AppName, void (*cb_arg)(char, char*)) {
	if (argv && argv[0]) {
		cb_arg('x', argv[0]);
		SetAppName(AppName, argv[0]);

		int i = 0;
		char* curarg;
		while ((curarg = argv[++i])) {
			cb_arg('x', curarg);
		}
	}
	cb_arg('\0', 0);
}
