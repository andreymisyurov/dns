#ifndef DNS_STRUCT_H
#define DNS_STRUCT_H

#include <json-c/json.h>
// Unterminated '#pragma pack (push, ...)' at end of file
#pragma pack(push, 1)
typedef struct DNS_HEADER{
    unsigned short id;

    unsigned char rd        :1;
    unsigned char tc        :1;
    unsigned char aa        :1;
    unsigned char opcode    :4;
    unsigned char qr        :1;

    unsigned char rcode     :4;
    unsigned char cd        :1;
    unsigned char ad        :1;
    unsigned char z         :1;
    unsigned char ra        :1;

    unsigned short q_count;
    unsigned short ancount;
    unsigned short nscount;
    unsigned short arcount;
} DNS_HEADER;
#pragma pack(pop)


typedef struct Config {
    const char* upstream_server;
    const char* error_response;
    json_object* blacklist;
} Config;

#endif // DNS_STRUCT_H