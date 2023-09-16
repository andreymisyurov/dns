#include <stdio.h>
#include <json-c/json.h>

// cooperate    https://www.geeksforgeeks.org/cjson-json-file-write-read-modify-in-c/
//              https://stackoverflow.com/questions/16975918/json-array-parsing-in-c

typedef struct Config {
    const char* upstream_server;
    const char* error_response;
    json_object* blacklist;
} Config;

int read_config(const char* in_filename, Config* out_config);
void print_config(const Config *in_config);