#include "parser/parser.h"
#include "net/net.h"

int main() {
    Config config;
    if (read_config("config.json", &config) != 0) {
        fprintf(stderr, "Failed to read configuration.\n");
        return 1;
    }

    print_config(&config);

    int port = 5353;
    int sock = initialize_socket(port);

    printf("DNS proxy server listening on port %d...\n", port);

    while (1) {
        request_client(sock, config.upstream_server);
    }

    close(sock);

    return 0;
}
