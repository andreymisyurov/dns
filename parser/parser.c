#include "parser.h"

int read_config(const char* in_filename, Config* out_config) {
    FILE *file = fopen(in_filename, "r");
    if (file == NULL) {
        perror("Failed to open config file");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = malloc(size + 1);
    fread(buffer, 1, size, file);
    buffer[size] = 0;

    json_object* parsed_json = json_tokener_parse(buffer);
    if (NULL == parsed_json) {
        fprintf(stderr, "Failed to parse JSON\n");
        free(buffer);
        fclose(file);
        return 1;
    }

    out_config->upstream_server = json_object_get_string(json_object_object_get(parsed_json, "upstream_server"));
    out_config->error_response = json_object_get_string(json_object_object_get(parsed_json, "error_response"));
    out_config->blacklist = json_object_object_get(parsed_json, "blacklist");

    free(buffer);
    fclose(file);
    return 0;
}