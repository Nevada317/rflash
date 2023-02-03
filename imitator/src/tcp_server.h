void PrintBuffer(char* tag, void* data, int size);
int server_start(int port, void (*cb)(void* data, int length));
void server_send(void* data, int length);
