#include <stdio.h>

#include "wold.h"
#include "wold_cmd_callback.h"

void print_found(void* callback_id) {
    printf("Found for cb %ld!\n", (long) callback_id);
}

uint8_t my_mac[]  = {1, 2, 3, 4, 5, 6};

int main() {
    struct wold_callback cb1 = {
            .mac = my_mac,
            .callback = wold_cmd_callback,
            .userarg = "date -I && exit 99",
    };

    struct wold* w = wold_new("br0");
    wold_add_callback(w, &cb1);
    int error = wold_start(w);

    printf("Result: %d\n", error);
}
