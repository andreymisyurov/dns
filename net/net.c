#include "net.h"

int initialize_socket(int port) {
    int sock = 0;
    struct sockaddr_in server_address = {
            .sin_family = AF_INET,
            .sin_addr.s_addr = INADDR_ANY,
            .sin_port = htons(port)
    };

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Error opening socket");
        return -1;
    }

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &((int){1}), sizeof(int)) < 0) {
        perror("Error setting socket options");
        close(sock);
        return -2;
    }

    if (bind(sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Error on binding");
        close(sock);
        return -2;
    }

    return sock;
}


void request_client(int in_sock) {
    char buffer[BUFFER_SIZE] = {0};
    struct sockaddr_in client_address;
    socklen_t client_len = sizeof(client_address);

    int n = recvfrom(in_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_address, &client_len);
    if (n < 0) {
        perror("Error on receiving");
        return;
    }

    n = sendto(in_sock, buffer, n, 0, (struct sockaddr*)&client_address, client_len);
    if (n < 0) {
        perror("Error on sending");
        return;
    }
}
