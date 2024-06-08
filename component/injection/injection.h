#ifndef INJECTION_H_H_H
#define INJECTION_H_H_H

#include "bt_type.h"

bt_ret_t bt_injection_open(uint8_t *file_name);
bt_ret_t bt_injection_write(uint8_t ble_channel, uint8_t *ble_access_address, uint8_t* ble_data, size_t ble_data_len);

#endif