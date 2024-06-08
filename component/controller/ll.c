#include "ll_pdu.h"
#include "ll.h"

bt_ret_t ll_parse_adv_pdu(uint8_t *data, uint16_t data_len)
{
    bdstr_t bdstr;
    ll_pdu_adv_t *ll_pdu_adv = (ll_pdu_adv_t *) data;

    LL_TRACE_DEBUG("T:%02x C:%d T:%d R:%d L:%d",ll_pdu_adv->type,ll_pdu_adv->chan_sel,
                    ll_pdu_adv->tx_addr,ll_pdu_adv->rx_addr,ll_pdu_adv->len);

    switch(ll_pdu_adv->type)
    {
        case PDU_ADV_TYPE_ADV_IND:
        {
            LL_TRACE_DEBUG("mac address: %s",bdaddr_to_string(ll_pdu_adv->adv_ind.addr, bdstr, sizeof(bdstr)));
            break;
        }
        case PDU_ADV_TYPE_DIRECT_IND:
        {

            break;
        }
        case PDU_ADV_TYPE_NONCONN_IND:
        {
            LL_TRACE_DEBUG("mac address: %s",bdaddr_to_string(ll_pdu_adv->adv_ind.addr, bdstr, sizeof(bdstr)));
            break;
        }
        case PDU_ADV_TYPE_SCAN_REQ:
        {

            break;
        }
        case PDU_ADV_TYPE_SCAN_RSP:
        {

            break;
        }
        case PDU_ADV_TYPE_CONNECT_IND:
        {
            break;
        }
        case PDU_ADV_TYPE_EXT_IND:
        {
            break;
        }
        default:
            LL_TRACE_WARNING("unsupport adv type:0x%x",ll_pdu_adv->type);
            break;
    }

    return BT_RET_SUCCESS;
}