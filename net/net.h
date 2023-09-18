/*
 * File: net.h
 * Author: Andrey Misyurov
 * Date: 15.09.23
 * Description: Network functions for working with sockets and DNS responses.
 */

#ifndef DNS_NET_NET_H_
#define DNS_NET_NET_H_

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
#define STANDARD_DNS_PORT 53
#define LOCAL_ADDR "192.168.0.15"

typedef bool (*func_ptr)(const Config *in_config, const char *in_name);

int initialize_socket(int port);
int send_local_address(DNS_HEADER *dns, ssize_t n);
int send_nxdomain(DNS_HEADER *dns, ssize_t n);
void handle_blocked_domain( int cl_sock,
                            char *buffer,
                            ssize_t *n,
                            const Config* conf,
                            struct sockaddr_in *cl_address,
                            socklen_t cl_addr_len);
ssize_t forward_to_dns_server(const char* ups_server, char* buffer, ssize_t n);
void request_client(int cl_sock, const Config* conf, func_ptr is_blocked);


#endif // DNS_NET_NET_H_