#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "wold_cmd_callback.h"

const char* default_shell = "/bin/sh";

void wold_cmd_callback(void* arg) {
    const char* cmd = arg;

    if (cmd == NULL || strlen(cmd) == 0)
        return;

    const char* shell = getenv("SHELL");
    if (shell == NULL)
        shell = default_shell;

    char shellrw[64];
    strncpy(shellrw, shell, sizeof(shellrw));

    char cmdrw[256];
    strncpy(cmdrw, arg, sizeof(cmdrw));

    char* const args[] = {
            shellrw,
            "-c",
            cmdrw,
            NULL,
    };

    if (fork() == 0) {
        execvp(shellrw, args);
        exit(1);
    }
}
