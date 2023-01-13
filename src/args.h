#ifndef _ARGS_H
#define _ARGS_H
#include <stdbool.h>

void ARGS_ParseArgs(char *argv[], char** AppName, bool (*cb_arg)(char, char*));

#endif /* _ARGS_H */
