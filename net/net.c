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




void request_client(int in_sock, const char* in_dns_server) {
    char buffer[BUFFER_SIZE] = {0};
    struct sockaddr_in client_address = {0};
    struct sockaddr_in dns_address = {
            .sin_family = AF_INET,
            .sin_port = htons(53),
            .sin_addr.s_addr = inet_addr(in_dns_server)
    };
    socklen_t client_len = sizeof(client_address);
    int n = recvfrom(in_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_address, &client_len);
    if (n < 0) {
        perror("Error on receiving");
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
    n = recvfrom(forward_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&dns_address, &client_len);
    if (n < 0) {
        perror("Error on receiving from DNS server");
        close(forward_sock);
        return;
    }
    close(forward_sock);
    n = sendto(in_sock, buffer, n, 0, (struct sockaddr*)&client_address, client_len);
    if (n < 0) {
        perror("Error on sending");
        return;
    }
}
