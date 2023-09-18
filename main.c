/*
 * File: main.c
 * Author: Andrey Misyurov
 * Date: 15.09.23
 * Description: Entry point for my DNS proxy server app.
 */

#include "parser/parser.h"
#include "net/net.h"

#define DEFAULT_PORT 53

int main() {
    Config config = {0};
    if (read_config("config.json", &config) != 0) {
        fprintf(stderr, "Failed to read configuration.\n");
        return 1;
    }

    print_config(&config);

    int port = DEFAULT_PORT;
    int sock = initialize_socket(port);

    printf("DNS proxy server listening on port %d...\n", port);

    while (1) {
        request_client(sock, &config, is_blocked);
    }

    close(sock);

    return 0;
}
