#include "net.h"

int initialize_socket(int in_port) {
    struct sockaddr_in server_address = {
            .sin_family = AF_INET,
            .sin_addr.s_addr = INADDR_ANY,
            .sin_port = htons(in_port)
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

void handle_blocked_domain(int in_sock, char *buffer, ssize_t *n, const Config* conf, struct sockaddr_in *client_address, socklen_t client_len) {
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
    *n = sendto(in_sock, buffer, response_size, 0, (struct sockaddr*)client_address, client_len);
}

void request_client(int in_sock, const Config* in_conf, func_ptr is_blocked) {
    char buffer[BUFFER_SIZE] = {0};
    struct sockaddr_in cl_address = {0};
    struct sockaddr_in dns_address = {
            .sin_family = AF_INET,
            .sin_port = htons(53),
            .sin_addr.s_addr = inet_addr(in_conf->upstream_server)
    };

    socklen_t cl_len = sizeof(cl_address);
    ssize_t n = recvfrom(in_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&cl_address, &cl_len);
    if (n < 0) {
        perror("Error on receiving");
        return;
    }

    char requested_name[BUFFER_SIZE] = {0};
    dn_expand((unsigned char*)buffer, (unsigned char*)buffer + n,
                        (const unsigned char*)buffer + DNS_HEADER_SIZE, requested_name, sizeof(requested_name));
    printf("Requested domain: %s\n", requested_name);

    if (is_blocked(in_conf, requested_name)) {
        printf("Domain %s is blocked:", requested_name);
        handle_blocked_domain(in_sock, buffer, &n, in_conf, &cl_address, cl_len);
        if (n < 0)
            perror("Error on sending response");
        return;
    }

    int forward_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (forward_sock < 0) {
        perror("Error opening forward socket");
        return;
    }
    setsockopt(forward_sock, SOL_SOCKET, SO_RCVTIMEO, &((struct timeval){.tv_sec = 3}), sizeof(struct timeval));
    n = sendto(forward_sock, buffer, n, 0, (struct sockaddr*)&dns_address, sizeof(dns_address));
    if (n < 0) {
        perror("Error on sending to DNS server");
        close(forward_sock);
        return;
    }
    n = recvfrom(forward_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&dns_address, &cl_len);
    if (n < 0) {
        perror("Error on receiving from DNS server");
        close(forward_sock);
        return;
    }
    close(forward_sock);
    n = sendto(in_sock, buffer, n, 0, (struct sockaddr*)&cl_address, cl_len);
    if (n < 0) {
        perror("Error on sending to client");
        return;
    }
}
