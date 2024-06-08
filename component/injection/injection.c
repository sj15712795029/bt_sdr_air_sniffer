#include "injection.h"
#include <stdlib.h>
#include <unistd.h>
#include <time.h>


FILE *injection_fd = NULL;

#define PCAP_MAGIC 0xa1b2c3d4
#define PCAP_MAJOR 2
#define PCAP_MINOR 4
#define PCAP_ZONE 0
#define PCAP_SIG 0
#define PCAP_SNAPLEN 0xffff
#define PCAP_BLE_LL_WITH_PHDR 256
#define BLE_FLAGS 0x3c37


#define NUM_BLE_CHANNELS 40

const int BLE_CHANS[NUM_BLE_CHANNELS] = {
    [37] = 0, [0] = 1, [1] = 2, [2] = 3, [3] = 4, [4] = 5, [5] = 6, [6] = 7, [7] = 8,
    [8] = 9, [9] = 10, [10] = 11, [38] = 12, [11] = 13, [12] = 14, [13] = 15, [14] = 16,
    [15] = 17, [16] = 18, [17] = 19, [18] = 20, [19] = 21, [20] = 22, [21] = 23, [22] = 24,
    [23] = 25, [24] = 26, [25] = 27, [26] = 28, [27] = 29, [28] = 30, [29] = 31, [30] = 32,
    [31] = 33, [32] = 34, [33] = 35, [34] = 36, [35] = 37, [36] = 38, [39] = 39
};

// Macro to convert channel
#define BLE_CHAN_MAP(channel) (BLE_CHANS[(channel)])


bt_ret_t bt_injection_open(uint8_t *file_name)
{
	injection_fd = fopen((const char *)file_name, "wb");
    if (!injection_fd) {
        return BT_RET_INVALID_ARG;
    }

    // Write PCAP file header
    uint32_t magic = PCAP_MAGIC;
    uint16_t version_major = PCAP_MAJOR;
    uint16_t version_minor = PCAP_MINOR;
    int32_t thiszone = PCAP_ZONE;
    uint32_t sigfigs = PCAP_SIG;
    uint32_t snaplen = PCAP_SNAPLEN;
    uint32_t network = PCAP_BLE_LL_WITH_PHDR;

    fwrite(&magic, sizeof(magic), 1, injection_fd);
    fwrite(&version_major, sizeof(version_major), 1, injection_fd);
    fwrite(&version_minor, sizeof(version_minor), 1, injection_fd);
    fwrite(&thiszone, sizeof(thiszone), 1, injection_fd);
    fwrite(&sigfigs, sizeof(sigfigs), 1, injection_fd);
    fwrite(&snaplen, sizeof(snaplen), 1, injection_fd);
    fwrite(&network, sizeof(network), 1, injection_fd);

    return BT_RET_SUCCESS;
}


bt_ret_t bt_injection_write(uint8_t ble_channel, uint8_t *ble_access_address, uint8_t* ble_data, size_t ble_data_len) 
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    uint32_t sec = ts.tv_sec;
    uint32_t usec = ts.tv_nsec / 1000;

    uint32_t ble_len = ble_data_len + 14;
    uint32_t incl_len = ble_len;
    uint32_t orig_len = ble_len;

    // Write PCAP packet header
    fwrite(&sec, sizeof(sec), 1, injection_fd);
    fwrite(&usec, sizeof(usec), 1, injection_fd);
    fwrite(&incl_len, sizeof(incl_len), 1, injection_fd);
    fwrite(&orig_len, sizeof(orig_len), 1, injection_fd);

    uint8_t channel = BLE_CHAN_MAP(ble_channel);
    uint8_t ff = 0xff;
    uint8_t zero = 0x00;
    uint16_t ble_flags = BLE_FLAGS;

    fwrite(&channel, sizeof(channel), 1, injection_fd);
    fwrite(&ff, sizeof(ff), 1, injection_fd);
    fwrite(&ff, sizeof(ff), 1, injection_fd);
    fwrite(&zero, sizeof(zero), 1, injection_fd);
    fwrite(ble_access_address, 4, 1, injection_fd);
    fwrite(&ble_flags, sizeof(ble_flags), 1, injection_fd);
    fwrite(ble_access_address, 4, 1, injection_fd);

    // Write BLE packet
    fwrite(ble_data, sizeof(uint8_t), ble_data_len, injection_fd);

    fflush(injection_fd);

	return BT_RET_SUCCESS;
}



