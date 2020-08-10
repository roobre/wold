#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wold_parser.h"

enum wold_parser_result parse_line(struct wold_config_parser* p, char* line, struct wold_callback* cb);
enum wold_parser_result alloclist_append(void** list, void*);

enum wold_parser_result wcp_open(struct wold_config_parser* p, const char* file) {
    p->alloclist[0] = NULL;
    p->fd = fopen(file, "r");

    if (p->fd == NULL)
        return WOLD_CONFIG_IOERR;

    return WOLD_CONFIG_OK;
}

enum wold_parser_result wcp_parse(struct wold_config_parser* p, struct wold_callback* cb) {
    char linebuf[1024];

    char* res = fgets(linebuf, sizeof(linebuf), p->fd);
    if (res == NULL)
        return WOLD_CONFIG_NOENTRY;

    return parse_line(p, linebuf, cb);
}

enum wold_parser_result parse_line(struct wold_config_parser* p, char* line, struct wold_callback* cb) {
    char* space = strchr(line, ' ');
    if (space == NULL)
        return WOLD_CONFIG_MALFORMED;

    space[0] = '\0';
    char* mac = line;
    char* cmd = space + 1;

    if (strlen(cmd) == 0)
        return WOLD_CONFIG_MALFORMED;

    cb->mac = malloc(6);
    alloclist_append(p->alloclist, cb->mac);

    char* macbyte = strtok(mac, ":");
    for (int i = 0; i < 6; i++) {
        if (macbyte == NULL)
            return WOLD_CONFIG_MALFORMED;
        if (strlen(macbyte) != 2)
            return WOLD_CONFIG_MALFORMED;

        char* invalid;
        cb->mac[i] = strtol(macbyte, &invalid, 16);
        if (*invalid != '\0')
            return WOLD_CONFIG_MALFORMED;

        macbyte = strtok(NULL, ":");
    }

    size_t cmdlen = strlen(cmd);
    cmd[cmdlen - 1] = '\0';

    cb->userarg = malloc(strlen(cmd));
    if (cb->userarg == NULL)
        return WOLD_CONFIG_IOERR;
    alloclist_append(p->alloclist, cb->userarg);

    strcpy(cb->userarg, cmd);

    return WOLD_CONFIG_OK;
}

void wcp_close(struct wold_config_parser* p) {
    for(int i = 0; p->alloclist[i] != NULL; i++)
        free(p->alloclist[i]);

    fclose(p->fd);
}

enum wold_parser_result alloclist_append(void** list, void* p) {
    int current = 0;
    while (list[current] != NULL)
        current++;

    if (current >= 62)
        return WOLD_CONFIG_NOENTRY;

    list[current] = p;
    list[current+1] = NULL;
}
