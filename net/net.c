/*
 * File: net.c
 * Author: Andrey Misyurov
 * Date: 15.09.23
 * Description: Network functions for managing sockets and processing DNS replies.
 */

#include "net.h"

/**
 * Initialize and bind UDP socket.
 * @param port: Port for binding socket.
 * @return: socket fd or error code.
 */
int initialize_socket(int port) {
    struct sockaddr_in server_address = {
            .sin_family = AF_INET,
            .sin_addr.s_addr = INADDR_ANY,
            .sin_port = htons(port)
    };

    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0) {
        perror("Error opening socket");
        return -1;
    }

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &((int){1}), sizeof(int)) < 0) {
        perror("Error setting socket options");
        close(sock);
        return -3;
    }

    if (bind(sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Error on binding");
        close(sock);
        return -2;
    }

    return sock;
}

/**
 * DNS response with local address.
 * @param dns: DNS structure pointer.
 * @param n: size of received DNS query.
 * @return: size of DNS response.
 */
int send_local_address(DNS_HEADER *dns, ssize_t n) {

    dns->qr         = 1;
    dns->aa         = 1;
    dns->rd         = 1;
    dns->ra         = 1;
    dns->rcode      = 0;
    dns->ancount    = htons(1);
    dns->nscount    = htons(0);
    dns->arcount    = htons(0);

    int response_len = (int)(sizeof(DNS_HEADER) + (n - sizeof(DNS_HEADER)));
    char *answer_ptr = (char *)(dns + 1) + (n - sizeof(DNS_HEADER));

    int question_length = (int)(n - sizeof(DNS_HEADER));
    memcpy(answer_ptr, dns + 1, question_length);

    answer_ptr += question_length;

    *(unsigned int *)answer_ptr = htonl(300);
    answer_ptr += 4;
    *(unsigned short *)answer_ptr = htons(4);
    answer_ptr += 2;
    *(unsigned int *)answer_ptr = inet_addr(LOCAL_ADDR);

    response_len += question_length + 10 + 4;
    return response_len;
}

/**
 * DNS response with NXDOMAIN error.
 * @param dns: DNS structure pointer.
 * @param n: size of received DNS query.
 * @return: size of DNS response.
 */
int send_nxdomain(DNS_HEADER *dns, ssize_t n) {
        dns->qr         = 1;
        dns->aa         = 1;
        dns->rd         = 1;
        dns->ra         = 1;
        dns->rcode      = 3;
        dns->ancount    = htons(0);
        dns->nscount    = htons(0);
        dns->arcount    = htons(0);
    return (int)(sizeof(DNS_HEADER) + (n - sizeof(DNS_HEADER)));
}

/**
 * Handles for sending responses for blocked domains.
 * @param cl_sock: client socket.
 * @param buffer: buffer with DNS query.
 * @param n: a pointer to variable with size of DNS query.
 * @param conf: configuration structure.
 * @param cl_address: client's address data.
 * @param cl_addr_len: len client's address struct.
 */
void handle_blocked_domain(int cl_sock, char *buffer, ssize_t *n, const Config* conf, struct sockaddr_in *cl_address, socklen_t cl_addr_len) {
    DNS_HEADER *dns = (DNS_HEADER *) buffer;
    int response_size = 0;
    if(strcmp(conf->error_response, "nxdomain") == 0) {
        printf(" sending NXDOMAIN...\n");
        response_size = send_nxdomain(dns, *n);
    } else if (strcmp(conf->error_response, "local address") == 0) {
        printf(" sending local address...\n");
        response_size = send_local_address(dns, *n);
    } else {
        printf(" not response...\n");
        return;
    }
    *n = sendto(cl_sock, buffer, response_size, 0, (struct sockaddr*)cl_address, cl_addr_len);
}

/**
 * Func forwards DNS query to real DNS server and receives a response.
 * @param ups_server: address of upstream DNS server.
 * @param buffer: buffer with DNS query and response.
 * @param n: size of DNS query.
 * @return: size of the received DNS response or error code.
 */
ssize_t forward_to_dns_server(const char* ups_server, char* buffer, ssize_t n) {
    struct sockaddr_in dns_address = {
            .sin_family = AF_INET,
            .sin_port = htons(STANDARD_DNS_PORT),
            .sin_addr.s_addr = inet_addr(ups_server)
    };

    int forward_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (forward_sock < 0) {
        perror("Error opening forward socket");
        return -1;
    }
    setsockopt(forward_sock, SOL_SOCKET, SO_RCVTIMEO, &((struct timeval){.tv_sec = 3}), sizeof(struct timeval));

    n = sendto(forward_sock, buffer, n, 0, (struct sockaddr*)&dns_address, sizeof(dns_address));
    if (n < 0) {
        perror("Error on sending to DNS server");
        close(forward_sock);
        return n;
    }

    socklen_t dns_len = sizeof(dns_address);
    n = recvfrom(forward_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&dns_address, &dns_len);
    close(forward_sock);
    if (n < 0) {
        perror("Error on receiving from DNS server");
    }
    return n;
}

/**
 * Full processing a domain which was requested by client
 * @param cl_sock: client socket fd.
 * @param conf: configuration object.
 * @param is_blocked: function pointer to check if domain is blocked. I did this function pointer,
 *                  because we can change format of configuration file. For example from json to xml
 *                  and we shouldn't change our logic.
 */
void request_client(int cl_sock, const Config* conf, func_ptr is_blocked) {
    char buffer[BUFFER_SIZE] = {0};
    struct sockaddr_in cl_address = {0};

    socklen_t cl_len = sizeof(cl_address);
    ssize_t n = recvfrom(cl_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&cl_address, &cl_len);
    if (n < 0) {
        perror("Error on receiving");
        return;
    }

    char requested_name[BUFFER_SIZE] = {0};
    dn_expand((unsigned char*)buffer, (unsigned char*)buffer + n,
                        (const unsigned char*)buffer + DNS_HEADER_SIZE, requested_name, sizeof(requested_name));
    printf("Requested domain: %s\n", requested_name);

    if (is_blocked(conf, requested_name)) {
        printf("Domain %s is blocked:", requested_name);
        handle_blocked_domain(cl_sock, buffer, &n, conf, &cl_address, cl_len);
        if (n < 0)
            perror("Error on sending response");
        return;
    }

    n = forward_to_dns_server(conf->upstream_server, buffer, n);
    if (n <= 0) return;

    n = sendto(cl_sock, buffer, n, 0, (struct sockaddr*)&cl_address, cl_len);
    if (n < 0) {
        perror("Error on sending to client");
        return;
    }
}
