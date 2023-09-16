#include <stdio.h>
#include "../struct.h"
#include <stdbool.h>

int read_config(const char* in_filename, Config* out_config);
void print_config(const Config *in_config);
bool is_blocked(const struct Config *in_config, const char *in_name);
