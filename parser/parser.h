/*
 * File: parser.h
 * Author: Andrey Misyurov
 * Date: 15.09.23
 * Description: Description: The necessary functions for working with json file.
 */

#ifndef DNS_PARSER_PARSER_H_
#define DNS_PARSER_PARSER_H_

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "../struct.h"

int read_config(const char* filename, Config* config);
void print_config(const Config *config);
bool is_blocked(const struct Config *config, const char *domain_name);

#endif // DNS_PARSER_PARSER_H_