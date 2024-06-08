#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include "bt_config.h"
#include "bt_type.h"
#include "bt_zmq_sub.h"
#include "ringbuffer.h"
#include "injection.h"
#include "ll.h"
#include "ll_pdu.h"

// TODO LIST
/** 
 * BLE SNIFFER topic
 * 1) CAN AD9361 capture different channel meantime, like capture adv channel 37/38/39 2402/2426/2480M Hz
 * 2）Send data to wireshark
 * 3）Parse LL Preamble/Access adddress
 * 4) dewhitening PDU data and CRC check
 * 5）Follow the frequency hopping
 * 6）Check other TODO item
 * 
*/

uint8_t bt_gfsk_buf[BT_GFSK_RING_BUF_SIZE];
struct ringbuffer bt_gfsk_ring_buf;

#define LL_UNCODED_PREAMBLE_1 0xAA
#define LL_UNCODED_PREAMBLE_2 0x55
#define LL_CODED_PREAMBLE 0x60
#define LL_CODED_PREABLE_LOOP_COUNT 10
#define LL_ACCESS_CODE_SIZE 4
#define LL_ADV_PACKET_HEADER_SIZE 2
#define LL_ADV_PACKET_PAYLOAD_MAX_SIZE 255
#define LL_PACKET_CRC_SIZE 3
#define LL_PACKET_PAYLOAD_MAX_SIZE 1024

typedef enum
{
    W4_LL_PREAMBLE,
    W4_LL_ACCESS_CODE,
	W4_LL_READ_HEADER,
	W4_LL_READ_PAYLOAD,
    W4_LL_CRC_CHECK,
    W4_DECRYPTION,
} ll_parse_packet_state_t;

typedef enum
{
	LL_PHY_UNKNOWN,
	LL_UNCODED_PHY_1M,
	LL_UNCODED_PHY_2M,
	LL_CODED_PHY,
} ll_parse_phy_t;

typedef enum
{
	LL_PACKET_TYPE_UNKNOWN,
	LL_PACKET_TYPE_ADV,
	LL_PACKET_TYPE_DATA,
} ll_packet_type_t;

typedef struct
{
	ll_parse_packet_state_t ll_parse_packet_state;

	uint8_t channel;
	uint8_t preamble;
	ll_packet_type_t ll_packet_type;
	ll_parse_phy_t ll_parse_phy;
	
	uint8_t access_code_pos;
	uint8_t access_code[LL_ACCESS_CODE_SIZE];

	uint8_t adv_payload_len;
	uint8_t adv_header[LL_ADV_PACKET_HEADER_SIZE];
	uint8_t adv_payload[LL_ADV_PACKET_PAYLOAD_MAX_SIZE];

	uint16_t payload_len;
	uint8_t packet_payload[LL_PACKET_PAYLOAD_MAX_SIZE];

} ll_parse_packet_cb_t;

ll_parse_packet_cb_t ll_parse_packet_cb = 
{
	.channel = 37,
	.preamble = 0,
	.ll_packet_type = LL_PACKET_TYPE_UNKNOWN,
	.ll_parse_phy = LL_PHY_UNKNOWN,
	.ll_parse_packet_state = W4_LL_PREAMBLE,
	.access_code_pos = 0,

	.adv_payload_len = 0,
	.payload_len = 0,
};

void ll_parse_packet_cb_reset()
{
	ll_parse_packet_cb.channel = 37;
	ll_parse_packet_cb.preamble = 0;
	ll_parse_packet_cb.ll_packet_type = LL_PACKET_TYPE_UNKNOWN;
	ll_parse_packet_cb.ll_parse_phy = LL_PHY_UNKNOWN;
	ll_parse_packet_cb.ll_parse_packet_state = W4_LL_PREAMBLE;
	ll_parse_packet_cb.access_code_pos = 0;
	ll_parse_packet_cb.adv_payload_len = 0;
	ll_parse_packet_cb.payload_len = 0;
}

void ll_set_preamble_type(uint8_t type)
{
	ll_parse_packet_cb.preamble = type;
}

uint8_t ll_get_preamble_type(void)
{
	return ll_parse_packet_cb.preamble;
}

void ll_set_packet_type(ll_packet_type_t type)
{
	ll_parse_packet_cb.ll_packet_type = type;
}

ll_packet_type_t ll_get_packet_type(void)
{
	return ll_parse_packet_cb.ll_packet_type;
}

void ll_set_phy(ll_parse_phy_t ll_parse_phy)
{
	ll_parse_packet_cb.ll_parse_phy = ll_parse_phy;
}

ll_parse_phy_t ll_get_phy(void)
{
	return ll_parse_packet_cb.ll_parse_phy;
}

void ll_set_parse_packet_state(ll_parse_packet_state_t ll_parse_packet_state)
{
	ll_parse_packet_cb.ll_parse_packet_state = ll_parse_packet_state;
}

ll_parse_packet_state_t ll_get_parse_packet_state(void)
{
	return ll_parse_packet_cb.ll_parse_packet_state;
}

// Swap bits in a byte
uint8_t bt_swap_bits(uint8_t value) {
    return (value * 0x0202020202ULL & 0x010884422010ULL) % 1023;
}


#define CRC24_POLY 0x00065B

void bt_crc(uint8_t *data, size_t len, uint32_t crc_init, uint8_t *crc_out) {
    uint8_t ret[3];
    ret[0] = (crc_init >> 16) & 0xff;
    ret[1] = (crc_init >> 8) & 0xff;
    ret[2] = crc_init & 0xff;

    for (size_t i = 0; i < len; i++) {
        uint8_t d = data[i];
        for (int v = 0; v < 8; v++) {
            uint8_t t = (ret[0] >> 7) & 1;

            ret[0] <<= 1;
            if (ret[1] & 0x80) {
                ret[0] |= 1;
            }

            ret[1] <<= 1;
            if (ret[2] & 0x80) {
                ret[1] |= 1;
            }

            ret[2] <<= 1;

            if ((d & 1) != t) {
                ret[2] ^= 0x5b;
                ret[1] ^= 0x06;
            }

            d >>= 1;
        }
    }

    crc_out[0] = bt_swap_bits(ret[0]);
    crc_out[1] = bt_swap_bits(ret[1]);
    crc_out[2] = bt_swap_bits(ret[2]);
}


// (De)Whiten data based on BLE channel, data is in/out param
void ll_packet_data_dewhitening(uint8_t *data, int length, uint8_t channel) {
    uint8_t lfsr = bt_swap_bits(channel) | 2;

    for (int i = 0; i < length; i++) {
        uint8_t d = bt_swap_bits(data[i]);
        for (int j = 128; j >= 1; j >>= 1) {
            if (lfsr & 0x80) {
                lfsr ^= 0x11;
                d ^= j;
            }
            lfsr <<= 1;
        }
        data[i] = bt_swap_bits(d);
    }

}



void zmp_data_ready_handler(void)
{
	uint8_t buffer[256] = {0};

	uint16_t read_len = bt_zmq_read_data(buffer,sizeof(buffer));
	//bt_hex_dump(buffer,read_len);

	// TODO: need find out other method avoid buffer multi copy, 
	// cause ringbuffer put function excute buffer copy inside
	if(ringbuffer_space_left(&bt_gfsk_ring_buf) > read_len)
		ringbuffer_put(&bt_gfsk_ring_buf,buffer,read_len);
}

void zmp_data_handler(void)
{
	switch(ll_get_parse_packet_state())
	{
		case W4_LL_PREAMBLE:
		{
			uint8_t ll_byte_0;
			uint8_t ll_byte_1;
			ringbuffer_get(&bt_gfsk_ring_buf,&ll_byte_0,1);
			switch(ll_byte_0)
			{
				case LL_UNCODED_PREAMBLE_1:
					ringbuffer_get(&bt_gfsk_ring_buf,&ll_byte_1,1);
					switch(ll_byte_1)
					{
						case LL_UNCODED_PREAMBLE_1:
							LL_PREPARSE_TRACE_DEBUG("2M phy data, preamble type is LL_UNCODED_PREAMBLE_1");
							ll_set_phy(LL_UNCODED_PHY_2M);
							break;
						case LL_UNCODED_PREAMBLE_2:
						case LL_CODED_PREAMBLE:
						default:
							LL_PREPARSE_TRACE_DEBUG("1M phy data, preamble type is LL_UNCODED_PREAMBLE_1");
							ll_set_phy(LL_UNCODED_PHY_1M);
							/* store this byte to parse access code */
							ll_parse_packet_cb.access_code[0] = ll_byte_1;
							ll_parse_packet_cb.access_code_pos = 1;
							break;
					}

					ll_set_preamble_type(LL_UNCODED_PREAMBLE_1);
					ll_set_parse_packet_state(W4_LL_ACCESS_CODE);

					break;
				case LL_UNCODED_PREAMBLE_2:
					ringbuffer_get(&bt_gfsk_ring_buf,&ll_byte_1,1);
					switch(ll_byte_1)
					{
						case LL_UNCODED_PREAMBLE_2:
							LL_PREPARSE_TRACE_DEBUG("2M phy data, preamble type is LL_UNCODED_PREAMBLE_2");
							ll_set_phy(LL_UNCODED_PHY_2M);
							break;
						case LL_UNCODED_PREAMBLE_1:
						case LL_CODED_PREAMBLE:
						default:
							LL_PREPARSE_TRACE_DEBUG("1M phy data, preamble type is LL_UNCODED_PREAMBLE_2");
							ll_set_phy(LL_UNCODED_PHY_1M);
							/* store this byte to parse access code */
							ll_parse_packet_cb.access_code[0] = ll_byte_1;
							ll_parse_packet_cb.access_code_pos = 1;
							break;
					}
					ll_set_preamble_type(LL_UNCODED_PREAMBLE_2);
					ll_set_parse_packet_state(W4_LL_ACCESS_CODE);
					break;
				case LL_CODED_PREAMBLE:
					// TODO: support codec phy
					LL_PREPARSE_TRACE_WARNING("Unsupport preamble ,maybe is codec phy");
					break;
				default:
					break;
			}
			break;
		}	
		case W4_LL_ACCESS_CODE:
		{
			if(ll_parse_packet_cb.access_code_pos == 0)
				ringbuffer_get(&bt_gfsk_ring_buf,ll_parse_packet_cb.access_code,LL_ACCESS_CODE_SIZE);
			else
				ringbuffer_get(&bt_gfsk_ring_buf,ll_parse_packet_cb.access_code + ll_parse_packet_cb.access_code_pos,
							LL_ACCESS_CODE_SIZE - ll_parse_packet_cb.access_code_pos);
			
			uint8_t lsb_data = ll_parse_packet_cb.access_code[LL_ACCESS_CODE_SIZE-1];
			bool valid_access_code = false;

			/* Check access code is match preamble */
			switch(ll_get_preamble_type())
			{
				case LL_UNCODED_PREAMBLE_1:
					if((lsb_data & 0x80) != 0)
						valid_access_code = true;
					break;
				case LL_UNCODED_PREAMBLE_2:
					if((lsb_data & 0x80) == 0)
						valid_access_code = true;
					break;
				case LL_CODED_PREAMBLE:
					// TODO: support codec phy
					break;
				default:
					break;
			}

			if(valid_access_code)
			{
				ll_set_parse_packet_state(W4_LL_READ_HEADER);
				if(ll_parse_packet_cb.access_code[0] == 0xD6 && ll_parse_packet_cb.access_code[1] == 0xBE && 
						ll_parse_packet_cb.access_code[2] == 0x89 && ll_parse_packet_cb.access_code[3] == 0x8e)
				{
					LL_PREPARSE_TRACE_DEBUG("Advertising packet type");
					ll_set_packet_type(LL_PACKET_TYPE_ADV);
				}
				else
				{
					LL_PREPARSE_TRACE_DEBUG("data packet type");
					ll_set_packet_type(LL_PACKET_TYPE_DATA);
				}
			}
			else
			{
				LL_PREPARSE_TRACE_ERROR("preamble and access code is not match");
				ll_parse_packet_cb_reset();
			}
			break;
		}
		case W4_LL_READ_HEADER:
		{
			ll_packet_type_t packet_type = ll_get_packet_type();
			if(packet_type == LL_PACKET_TYPE_ADV)
			{
				uint8_t adv_header[LL_ADV_PACKET_HEADER_SIZE];
				ringbuffer_get(&bt_gfsk_ring_buf,adv_header,LL_ADV_PACKET_HEADER_SIZE);
				memcpy(ll_parse_packet_cb.packet_payload,adv_header,LL_ADV_PACKET_HEADER_SIZE);
				ll_packet_data_dewhitening(adv_header,LL_ADV_PACKET_HEADER_SIZE,ll_parse_packet_cb.channel);
				ll_parse_packet_cb.adv_payload_len = adv_header[1];
				LL_PREPARSE_TRACE_DEBUG("Advertising packet payload len:%d",ll_parse_packet_cb.adv_payload_len);
				ll_set_parse_packet_state(W4_LL_READ_PAYLOAD);

			}
			else if(packet_type == LL_PACKET_TYPE_DATA)
			{
				// TODO : parse data packet type
				ll_parse_packet_cb_reset();
			}
			break;
		}	
		case W4_LL_READ_PAYLOAD:
		{
			ll_packet_type_t packet_type = ll_get_packet_type();
			if(packet_type == LL_PACKET_TYPE_ADV)
			{
				if(ringbuffer_len(&bt_gfsk_ring_buf) < ll_parse_packet_cb.adv_payload_len)
				{
					LL_PREPARSE_TRACE_WARNING("bt_gfsk_ring_buf len: %d<%d\n",ringbuffer_len(&bt_gfsk_ring_buf),ll_parse_packet_cb.adv_payload_len);
					return;
				}
				ringbuffer_get(&bt_gfsk_ring_buf,ll_parse_packet_cb.packet_payload + LL_ADV_PACKET_HEADER_SIZE,ll_parse_packet_cb.adv_payload_len);
				ll_parse_packet_cb.payload_len = LL_ADV_PACKET_HEADER_SIZE + ll_parse_packet_cb.adv_payload_len;

				ll_set_parse_packet_state(W4_LL_CRC_CHECK);

			}
			else if(packet_type == LL_PACKET_TYPE_DATA)
			{
				// TODO : parse data packet type
				ll_parse_packet_cb_reset();
			}
			break;
		}			
		case W4_LL_CRC_CHECK:
		{

			if(ringbuffer_len(&bt_gfsk_ring_buf) < LL_PACKET_CRC_SIZE)
			{
				LL_PREPARSE_TRACE_WARNING("bt_gfsk_ring_buf len: %d<%d\n",ringbuffer_len(&bt_gfsk_ring_buf),LL_PACKET_CRC_SIZE);
				return;
			}
			ringbuffer_get(&bt_gfsk_ring_buf,ll_parse_packet_cb.packet_payload + ll_parse_packet_cb.payload_len,LL_PACKET_CRC_SIZE);
			ll_parse_packet_cb.payload_len += LL_PACKET_CRC_SIZE;
			ll_packet_data_dewhitening(ll_parse_packet_cb.packet_payload,ll_parse_packet_cb.payload_len,ll_parse_packet_cb.channel);
			
			uint8_t calc_crc[LL_PACKET_CRC_SIZE];
			bt_crc(ll_parse_packet_cb.packet_payload,ll_parse_packet_cb.payload_len - LL_PACKET_CRC_SIZE,PDU_AC_CRC_IV,calc_crc);

			if(memcmp(calc_crc,ll_parse_packet_cb.packet_payload + ll_parse_packet_cb.payload_len - LL_PACKET_CRC_SIZE,LL_PACKET_CRC_SIZE) == 0)
			{
				LL_PREPARSE_TRACE_DEBUG("Complete and correct packet,crc check is pass");
				ll_parse_adv_pdu(ll_parse_packet_cb.packet_payload,ll_parse_packet_cb.payload_len);
				bt_injection_write(ll_parse_packet_cb.channel,ll_parse_packet_cb.access_code,
										ll_parse_packet_cb.packet_payload,ll_parse_packet_cb.payload_len);
			}
			else
			{
				LL_PREPARSE_TRACE_ERROR("Uncomplete or uncorrent packet,crc check is fail");		
			}

			// TODO: What situations need to be decrypted
			ll_parse_packet_cb_reset();
			break;
		}		
		case W4_DECRYPTION:
			break;
		default:
			break;
	}
	
}

int main(int argc, char *argv[])
{
	int opt;
	char *output_file = NULL;

	while ((opt = getopt(argc, argv, "o:")) != -1) {
        switch (opt) {
            case 'o':
                output_file = optarg;
                break;
            default: /* '?' */
                break;
        }
    }

    if (output_file == NULL) {
        LL_PREPARSE_TRACE_ERROR("output_file is null");	
        return -1;
    }

	ringbuffer_init(&bt_gfsk_ring_buf,bt_gfsk_buf,BT_GFSK_RING_BUF_SIZE);
	bt_injection_open((uint8_t *)output_file);
	bt_zmq_subscriber();
	bt_zmq_start_monitor_data(zmp_data_ready_handler);

	while(1)
	{
		if(!ringbuffer_is_empty(&bt_gfsk_ring_buf))
			zmp_data_handler();
		else
			usleep(1000);
	}
	

	return 0;
}
