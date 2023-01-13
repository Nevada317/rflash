#include "args.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void SetAppName(char** AppName, char* value) {
	if (!AppName) return;
	if (*AppName) {
		free(*AppName);
		*AppName = 0;
	}
	if (value && strlen(value)) {
		*AppName = strdup(value);
	}
}

void ARGS_ParseArgs(char *argv[], char** AppName, bool (*cb_arg)(char, char*)) {
	if (argv && argv[0]) {
		SetAppName(AppName, argv[0]);

		int i = 0;
		int j = 0;
		char* curarg;
		char letter = ' ';
		while ((curarg = argv[++i])) {
			// printf("Next arg: %s\n", curarg);
			if (curarg[0]=='-') {
				j = 0;
				while ((letter = curarg[++j])) {
					if (!cb_arg(letter, 0)) {
						break;
					}
				}
			}
			if (letter) {
				curarg = argv[++i];
				if (!curarg) break;
				// If next argument starts with dash
				if (curarg[0] == '-') {
					// Undo arg consumption
					i--;
				} else {
					cb_arg(letter, curarg);
				}
			}
			letter = ' ';
		}
	}
	cb_arg('\0', 0);
}
