#include <stdbool.h>

bool connect_tcp_qualified(char* address, int port, void (*cb)(void* data, int length));
void disconnect_tcp();
void server_send(void* data, int length);
