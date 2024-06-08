#ifndef BT_TYPE_H_H_H
#define BT_TYPE_H_H_H

#include <assert.h>
#include "bt_log.h"

#define BDADDR_SIZE   6
typedef uint8_t BD_ADDR[BDADDR_SIZE];

typedef enum
{
    BT_RET_SUCCESS,
    BT_RET_INVALID_ARG,
    BT_RET_DISALLOWED,
    BT_RET_UNSUPPORT,
    BT_RET_LIMITED_RESOURCES,
} bt_ret_t;

typedef char bdstr_t[sizeof("xx:xx:xx:xx:xx:xx")];

#define BT_PACK_END	__attribute__ ((packed))

static const inline char *bdaddr_to_string(const BD_ADDR addr, char *string, size_t size)
{
    assert(addr != NULL);
    assert(string != NULL);

    if (size < 18u) {
        return NULL;
    }

    const uint8_t *ptr = addr;
    (void) sprintf(string, "%02x:%02x:%02x:%02x:%02x:%02x",
                   ptr[0], ptr[1], ptr[2],
                   ptr[3], ptr[4], ptr[5]);
    return string;
}

#endif