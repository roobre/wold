#pragma once

#include <stdio.h>
#include "wold.h"

enum wold_parser_result {
    WOLD_CONFIG_OK,
    WOLD_CONFIG_MALFORMED,
    WOLD_CONFIG_NOENTRY,
    WOLD_CONFIG_IOERR,
};

struct wold_config_parser {
    FILE* fd;
    void* alloclist[64];
};

enum wold_parser_result wcp_open(struct wold_config_parser* p, const char* file);
enum wold_parser_result wcp_parse(struct wold_config_parser* p, struct wold_callback* cb);
void wcp_close(struct wold_config_parser* p);
