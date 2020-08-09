#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <net/ethernet.h>

#include "wold.h"

#define MAX_CB_NUM 16

struct wold {
    const char* interface;
    struct wold_callback callbacks[MAX_CB_NUM];
    int num_callbacks;
};

static void parse_packet(struct wold* w, const uint8_t* packetbuf, size_t buflen);
static int find_and_dispatch(struct wold* w, const uint8_t* magic);

static int is_wol_for_host(const uint8_t* magic, const uint8_t* mac);

static void print_mac(const uint8_t* mac);

struct wold* wold_new(const char* interface) {
    struct wold* w = calloc(1, sizeof(struct wold));
    if (w == NULL)
        return NULL;

    w->interface = interface;

    return w;
}

void wold_free(struct wold* w) {
    free(w);
}

enum wold_result wold_add_callback(struct wold* w, const struct wold_callback* cb) {
    if (w->num_callbacks == MAX_CB_NUM)
        return WOLD_RESULT_FULL;

    memcpy(&w->callbacks[w->num_callbacks], cb, sizeof(struct wold_callback));

    w->num_callbacks++;

    return WOLD_RESULT_OK;
}

enum wold_result wold_start(struct wold* w) {
    int interface = if_nametoindex(w->interface);
    if (interface == 0)
        return WOLD_IF_NOT_FOUND;

    int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sock == -1)
        return errno;

    struct sockaddr_ll bindaddr = {
            .sll_family = AF_PACKET,
            .sll_pkttype = PACKET_BROADCAST,
            .sll_ifindex = interface,
    };

    int bindres = bind(sock, &bindaddr, sizeof(bindaddr));
    if (bindres != 0)
        return errno;

    uint8_t sockbuf[2*1024];
    do {
        recv(sock, sockbuf, sizeof(sockbuf), 0);
        parse_packet(w, sockbuf, sizeof(sockbuf));
    } while (errno == 0);

    return errno;
}

static void parse_packet(struct wold* w, const uint8_t* packetbuf, size_t buflen) {
    if (buflen <= sizeof(struct ether_header))
        return;

    const uint8_t ethbcast[ETH_ALEN] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,};
    const struct ether_header* ethhdr = (struct ether_header*) packetbuf;

    if (memcmp(ethhdr->ether_dhost, ethbcast, ETH_ALEN) != 0)
        return;

    const size_t payload_len = buflen - sizeof(struct ether_header);
    const uint8_t* payload = packetbuf + sizeof(struct ether_header);

    const uint8_t* mpacket = payload;
    for (;;) {
        mpacket = memchr(mpacket, 0xFF, payload_len);
        if (mpacket == NULL)
            return;

        size_t matchsize = mpacket - payload;
        if (buflen - matchsize < 17 * ETH_ALEN)
            return;

        if (memcmp(mpacket, ethbcast, ETH_ALEN) != 0) {
            mpacket++;
            continue;
        }

        // Call is_wol_for_host with buffer containing potential mac repetitions
        if (find_and_dispatch(w, mpacket + ETH_ALEN) != 0)
            return;

        mpacket++;
    };
}

static int find_and_dispatch(struct wold* w, const uint8_t* magic) {
    for (int h = 0; h < w->num_callbacks; h++) {
        struct wold_callback* callback = &w->callbacks[h];

        if (is_wol_for_host(magic, callback->mac)) {
            printf("Found magic packet for host ");
            print_mac(callback->mac);
            puts("");
            callback->callback(callback->userarg);

            return 1;
        }
    }

    return 0;
}

static int is_wol_for_host(const uint8_t* magic, const uint8_t* mac) {
    for (int i = 0; i < 16; i++) {
        if (memcmp(mac, magic + ETH_ALEN * i, ETH_ALEN) != 0) {
            return 0;
        }
    }

    return 1;
}

static void print_mac(const uint8_t* mac) {
    for (int i = 0; i < ETH_ALEN - 1; i++)
        printf("%02x:", mac[i]);
    printf("%02x", mac[ETH_ALEN - 1]);
}
