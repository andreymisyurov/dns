/*
 * File: parser.c
 * Author: Andrey Misyurov
 * Date: 15.09.23
 * Description: The necessary functions for working with json file.
 */

#include "parser.h"

/**
 * Func for parsing JSON file.
 *
 * @param filename path to json file with configuration.
 * @param config out-argument. A pointer on structure of configuration.
 * @return 0 - successful. 1 - reading was failed.
 */
int read_config(const char* filename, Config* config) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open config file");
        return 1;
    }
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* buffer = malloc(size + 1);
    if (!buffer) {
        perror("Failed to allocate memory");
        fclose(file);
        return 1;
    }

    size_t bytes_read = fread(buffer, 1, size, file);
    if (bytes_read != size) {
        perror("Failed to read config file");
        free(buffer);
        fclose(file);
        return 1;
    }
    buffer[size] = 0;

    json_object* parsed_json = json_tokener_parse(buffer);
    if (NULL == parsed_json) {
        fprintf(stderr, "Failed to parse JSON\n");
        free(buffer);
        fclose(file);
        return 1;
    }
    config->upstream_server = json_object_get_string(json_object_object_get(parsed_json, "upstream_server"));
    config->error_response = json_object_get_string(json_object_object_get(parsed_json, "error_response"));
    config->blacklist = json_object_object_get(parsed_json, "blacklist");
    free(buffer);
    fclose(file);
    return 0;
}

/**
 * Printing a configuration to console.
 *
 * @param config A pointer on structure of configuration
 */
void print_config(const Config *config) {
    printf("Upstream Server: %s\n", config->upstream_server);
    printf("Error Response: %s\n", config->error_response);
    size_t blacklist_length = json_object_array_length(config->blacklist);
    printf("Blacklist:\n");
    for (int i = 0; i < blacklist_length; ++i) {
        printf("- %s\n", json_object_get_string(json_object_array_get_idx(config->blacklist, i)));
    }
}

/**
 * Check domain into the blacklist.
 *
 * @param config Pointer to the Config structure containing the blacklist.
 * @param domain_name The domain name to check.
 * @return true(domain name is blacked), false(domain name is resolved).
 */
bool is_blocked(const struct Config *config, const char *domain_name) {
    if (!config->blacklist) {
        return false;
    }

    size_t blacklist_length = json_object_array_length(config->blacklist);
    for (int i = 0; i < blacklist_length; ++i) {
        const char *blacklisted_domain = json_object_get_string(json_object_array_get_idx(config->blacklist, i));
        if (0 == strcmp(domain_name, blacklisted_domain)) {
            return true;
        }
    }

    return false;
}
