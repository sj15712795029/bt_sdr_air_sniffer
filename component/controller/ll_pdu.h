#ifndef LL_PDU_H_H_H
#define LL_PDU_H_H_H

#include "bt_type.h"

/* Channel Map Size */
#define PDU_CHANNEL_MAP_SIZE 5
/* Advertisement channel maximum legacy advertising/scan data size */
#define PDU_AC_LEG_DATA_SIZE_MAX   31
/* Advertisement channel CRC init value */
#define PDU_AC_CRC_IV 0x555555

#define LL_ADDR_TYPE_PUBLIC 0
#define LL_ADDR_TYPE_RANDOM 1

typedef enum  {
	PDU_ADV_TYPE_ADV_IND = 0x00,
	PDU_ADV_TYPE_DIRECT_IND = 0x01,
	PDU_ADV_TYPE_NONCONN_IND = 0x02,
	PDU_ADV_TYPE_SCAN_REQ = 0x03,
	PDU_ADV_TYPE_AUX_SCAN_REQ = PDU_ADV_TYPE_SCAN_REQ,
	PDU_ADV_TYPE_SCAN_RSP = 0x04,
	PDU_ADV_TYPE_ADV_IND_SCAN_RSP = 0x05,
	PDU_ADV_TYPE_CONNECT_IND = 0x05,
	PDU_ADV_TYPE_AUX_CONNECT_REQ = PDU_ADV_TYPE_CONNECT_IND,
	PDU_ADV_TYPE_SCAN_IND = 0x06,
	PDU_ADV_TYPE_EXT_IND = 0x07,
	PDU_ADV_TYPE_AUX_ADV_IND = PDU_ADV_TYPE_EXT_IND,
	PDU_ADV_TYPE_AUX_SCAN_RSP = PDU_ADV_TYPE_EXT_IND,
	PDU_ADV_TYPE_AUX_SYNC_IND = PDU_ADV_TYPE_EXT_IND,
	PDU_ADV_TYPE_AUX_CHAIN_IND = PDU_ADV_TYPE_EXT_IND,
	PDU_ADV_TYPE_AUX_CONNECT_RSP = 0x08,
} ll_pdu_adv_type_t;

typedef struct {
	uint8_t addr[BDADDR_SIZE];
	uint8_t data[PDU_AC_LEG_DATA_SIZE_MAX];
} BT_PACK_END ll_pdu_adv_adv_ind_t;

typedef struct {
	uint8_t adv_addr[BDADDR_SIZE];
	uint8_t tgt_addr[BDADDR_SIZE];
} BT_PACK_END ll_pdu_adv_direct_ind_t;

typedef struct {
	uint8_t addr[BDADDR_SIZE];
	uint8_t data[PDU_AC_LEG_DATA_SIZE_MAX];
} BT_PACK_END ll_pdu_adv_scan_rsp_t;

typedef struct {
	uint8_t scan_addr[BDADDR_SIZE];
	uint8_t adv_addr[BDADDR_SIZE];
} BT_PACK_END ll_pdu_adv_scan_req_t;

typedef struct {
	uint8_t init_addr[BDADDR_SIZE];
	uint8_t adv_addr[BDADDR_SIZE];
	struct {
		uint8_t  access_addr[4];
		uint8_t  crc_init[3];
		uint8_t  win_size;
		uint16_t win_offset;
		uint16_t interval;
		uint16_t latency;
		uint16_t timeout;
		uint8_t  chan_map[PDU_CHANNEL_MAP_SIZE];
		uint8_t  hop:5;
		uint8_t  sca:3;
	} BT_PACK_END;
} BT_PACK_END ll_pdu_adv_connect_ind_t;

typedef struct {
	uint8_t adv_addr:1;
	uint8_t tgt_addr:1;
	uint8_t cte_info:1;
	uint8_t adi:1;
	uint8_t aux_ptr:1;
	uint8_t sync_info:1;
	uint8_t tx_pwr:1;
	uint8_t rfu:1;
	uint8_t data[0];
} BT_PACK_END ll_pdu_adv_ext_hdr_t;

typedef struct {
	uint8_t ext_hdr_len:6;
	uint8_t adv_mode:2;

	union {
		ll_pdu_adv_ext_hdr_t ext_hdr;
		uint8_t ext_hdr_adv_data[0];
	} BT_PACK_END;
} BT_PACK_END ll_pdu_adv_com_ext_adv_t;

/*
 * Advertising physical channel PDU header
 * 
 *  +--------+-----+-------+--------+--------+--------+
 *  | PDU    | RFU | ChSel | TxAdd  | RxAdd  | Length |
 *  | Type   | (1  | (1    | (1     | (1     | (8     |
 *  | (4     | bit)| bit)  | bit)   | bit)   | bits)  |
 *  +--------+-----+-------+--------+--------+--------+
 * 
 * PDU Type: Indicates the type of PDU (4 bits)
 * RFU: Reserved for Future Use (1 bit)
 * ChSel: Channel Selection (1 bit)
 * TxAdd: Transmitter Address Type (1 bit)
 * RxAdd: Receiver Address Type (1 bit)
 * Length: Length of the payload in bytes (8 bits)
 */

typedef struct {
	uint8_t type:4;
	uint8_t rfu:1;
	uint8_t chan_sel:1;
	uint8_t tx_addr:1;
	uint8_t rx_addr:1;

	uint8_t len;

    union {
		uint8_t   payload[0];
		ll_pdu_adv_adv_ind_t adv_ind;
		ll_pdu_adv_direct_ind_t direct_ind;
		ll_pdu_adv_scan_req_t scan_req;
		ll_pdu_adv_scan_rsp_t scan_rsp;
		ll_pdu_adv_connect_ind_t connect_ind;
        ll_pdu_adv_com_ext_adv_t adv_ext_ind;
	} BT_PACK_END;

} BT_PACK_END ll_pdu_adv_t;

#endif