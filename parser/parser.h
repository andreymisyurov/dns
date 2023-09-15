#include <stdio.h>
#include <json-c/json.h>

typedef struct Config {
    const char* upstream_server;
    const char* error_response;
    json_object* blacklist;
} Config;

int read_config(const char* in_filename, Config* out_config);