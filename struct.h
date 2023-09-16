#ifndef _STRUCT_H
#define _STRUCT_H

#include <json-c/json.h>

typedef struct Config {
    const char* upstream_server;
    const char* error_response;
    json_object* blacklist;
} Config;

#endif // _STRUCT_H