#include "parser/parser.h"

int main() {
    Config config;
    if (read_config("config.json", &config) == 0) {
        printf("Upstream Server: %s\n", config.upstream_server);
        printf("Error Response: %s\n", config.error_response);

        int blacklist_length = json_object_array_length(config.blacklist);
        printf("Blacklist:\n");
        for (int i = 0; i < blacklist_length; i++) {
            printf("- %s\n", json_object_get_string(json_object_array_get_idx(config.blacklist, i)));
        }
    } else {
        fprintf(stderr, "Failed to read configuration.\n");
    }

    return 0;
}
