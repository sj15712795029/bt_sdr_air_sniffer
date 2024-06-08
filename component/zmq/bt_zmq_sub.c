#include <zmq.h>
#include <pthread.h>
#include "bt_zmq_sub.h"

void *zmq_ctx_context = NULL;
void *zmq_subscriber = NULL;


bt_ret_t bt_zmq_subscriber(void)
{
	void *zmq_ctx_context = zmq_ctx_new();
	if (zmq_ctx_context == NULL) {
        ZMP_TRACE_ERROR("Failed to create ZeroMQ context");
        return BT_RET_DISALLOWED;
    }
	
	zmq_subscriber = zmq_socket(zmq_ctx_context, ZMQ_SUB);
    if (zmq_subscriber == NULL) {
        ZMP_TRACE_ERROR("Failed to create ZeroMQ subscriber socket");
        zmq_ctx_destroy(zmq_ctx_context);
        zmq_ctx_context = NULL;
        return BT_RET_DISALLOWED;
    }
	

	int rc = zmq_connect(zmq_subscriber, "tcp://127.0.0.1:55555");
	if (rc != 0)
	{
		ZMP_TRACE_ERROR("zmq_connect fail :0x%x", rc);
		return BT_RET_DISALLOWED;
	}

	/* subscriber all topic */
	zmq_setsockopt(zmq_subscriber, ZMQ_SUBSCRIBE, "", 0);

	return BT_RET_SUCCESS;
}

bt_ret_t bt_zmq_unsubscriber(void)
{
	if(zmq_subscriber == NULL || zmq_ctx_context == NULL)
	{
		ZMP_TRACE_WARNING("zmq_subscriber or zmq_ctx_context is NULL");
		return BT_RET_SUCCESS;
	}
	zmq_close(zmq_subscriber);
    zmq_ctx_destroy(zmq_ctx_context);

	zmq_subscriber = NULL;
    zmq_ctx_context = NULL;

	return BT_RET_SUCCESS;
}

static void* zmq_data_monitor_thread(void* arg) 
{
    zmp_data_ready_handler_t data_ready_handler = (zmp_data_ready_handler_t)arg;
    while(1)
	{
		zmq_pollitem_t items[] = {
        	{ zmq_subscriber, 0, ZMQ_POLLIN, 0 }
    	};

		zmq_poll(items, 1, -1);

		if (items[0].revents & ZMQ_POLLIN)
		{
			data_ready_handler();
		}
	}
    return NULL;
}

bt_ret_t bt_zmq_start_monitor_data(zmp_data_ready_handler_t handler)
{
	pthread_t start_monitor_thread_id;
	pthread_create(&start_monitor_thread_id, NULL, zmq_data_monitor_thread, handler);
	return BT_RET_SUCCESS;
}

uint16_t bt_zmq_read_data(uint8_t *data, uint16_t read_len)
{
	return zmq_recv(zmq_subscriber, data, read_len-1, 0);
}



