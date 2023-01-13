#include "args.h"
#include <stdlib.h>
#include <string.h>

static arg_key_t* KeysBuffer = NULL;

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
			if (curarg[0]=='-') {
				j = 0;
				while ((letter = curarg[++j])) {
					if (!cb_arg(letter, 0)) {
						break;
					}
				}
			}
			if (letter) {
				if (curarg[j] && curarg[j+1]) {
					curarg = curarg+j+1;
				} else {
					// Consume next argument in list
					curarg = argv[++i];
					if (!curarg) break;
					// If next argument starts with dash
					if (curarg[0] == '-') {
						// Undo arg consumption
						i--;
						curarg = 0;
					}
				}
				if (curarg && curarg[0]) {
					cb_arg(letter, curarg);
				}
			}
			letter = ' ';
		}
	}
	cb_arg('\0', 0);
}

static bool _ARGS_InternalCallback(char key, char* arg) {
	if (!KeysBuffer) return true;
	int i = 0;
	arg_key_t* ptr;
	while ((ptr = &KeysBuffer[i++])->handler) {
		if (ptr->key == key) {
			if (ptr->needs_arg && !arg) return false;
			if (ptr->handler) ptr->handler(key, arg);
			return true;
		}
	}
	// Tag not Found in list, default to 0'th item
	ptr = &KeysBuffer[0];
	if (ptr->handler) ptr->handler(key, arg);
	return true;
}

void ARGS_ParseArgsByList(char *argv[], char** AppName, arg_key_t Keys[]) {
	KeysBuffer = Keys;
	if (!KeysBuffer) return;
	ARGS_ParseArgs(argv, AppName, _ARGS_InternalCallback);
	KeysBuffer = NULL;
}
