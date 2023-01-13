#ifndef _ARGS_H
#define _ARGS_H
#include <stdbool.h>

typedef struct {
	char key;
	bool needs_arg;
	void (*handler)(char key, char* arg);
} arg_key_t;

void ARGS_ParseArgs(char *argv[], char** AppName, bool (*cb_arg)(char, char*));
void ARGS_ParseArgsByList(char *argv[], char** AppName, arg_key_t Keys[]);

#endif /* _ARGS_H */
