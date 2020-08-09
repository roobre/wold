#pragma once

#include <stdint.h>

typedef void(*wold_callback)(void*);

struct wold;

enum wold_result {
    WOLD_RESULT_OK,
    WOLD_IF_NOT_FOUND,
    WOLD_RESULT_FULL,
};

struct wold_callback {
    uint8_t* mac;
    wold_callback callback;
    void* userarg;
};

struct wold* wold_new(const char* interface);
void wold_free(struct wold* w);

enum wold_result wold_add_callback(struct wold* w, const struct wold_callback* cb);
enum wold_result wold_start(struct wold* w);
