#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <resolv.h>
#include <stdbool.h>
#include "../struct.h"


#define BUFFER_SIZE 512
#define DNS_HEADER_SIZE 12

typedef bool (*func_ptr)(const Config *in_config, const char *in_name);

int initialize_socket(int in_port);
void request_client(int in_sock, const Config* conf, func_ptr is_blocked);
