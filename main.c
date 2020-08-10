#include <stdio.h>

#include "wold.h"
#include "wold_cmd_callback.h"
#include "wold_parser.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <interface> [config.txt]\n", argv[0]);
        return 1;
    }

    const char* configfile;
    if (argc < 3) {
        configfile = "config.txt";
    } else {
        configfile = argv[2];
    }

    struct wold* w = wold_new(argv[1]);

    struct wold_config_parser parser;
    printf("Loading confif file from %s\n", configfile);
    if (wcp_open(&parser, configfile) != WOLD_CONFIG_OK)
        return 1;

    for (;;) {
        struct wold_callback cb;
        cb.callback = wold_cmd_callback;
        enum wold_parser_result res = wcp_parse(&parser, &cb);
        if (res == WOLD_CONFIG_NOENTRY)
            break;
        if (res == WOLD_CONFIG_MALFORMED)
            continue;

        printf("Registered callback '%s' to MAC %02x\n", cb.userarg, cb.mac[5]);

        wold_add_callback(w, &cb);
    }

    wold_start(w);
    wcp_close(&parser);
}
