#ifndef BT_ZMQ_SUB_H_H_H
#define BT_ZMQ_SUB_H_H_H

#include "bt_type.h"

typedef void (*zmp_data_ready_handler_t)(void);

bt_ret_t bt_zmq_subscriber(void);
bt_ret_t bt_zmq_unsubscriber(void);
bt_ret_t bt_zmq_start_monitor_data(zmp_data_ready_handler_t handler);
uint16_t bt_zmq_read_data(uint8_t *data, uint16_t read_len);

#endif