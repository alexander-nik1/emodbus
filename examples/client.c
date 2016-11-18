
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>

#include <event2/event.h>

#include <emodbus/base/common.h>
#include <emodbus/base/modbus_transport.h>
#include <emodbus/base/modbus_errno.h>

#include <emodbus/client/client.h>
#include <emodbus/client/read_regs.h>

#include <emodbus/impl/posix/mb-rtu-via-serial.h>
#include <emodbus/impl/posix/dumper.h>
#include <emodbus/impl/posix/client.h>

/***************************************************************************************************/
/*                                                                                                 */
/*                                SYNCHRONOUS CLIENT EXAMPLE                                       */
/*                                                                                                 */
/***************************************************************************************************/

/***************************************************************************************************/
// Synchronous client
struct emb_posix_sync_client_t* client;

/***************************************************************************************************/
// This function are called at Ctrl+C

static void signal_cb(evutil_socket_t sig, short events, void *user_data)
{
    struct event_base *base = user_data;
    printf("Caught an interrupt signal; exiting cleanly.\n");
    event_base_loopexit(base, NULL);
}

/***************************************************************************************************/
// Thread for a transport-level operations.
// This thread works with a physical port.
void* thr_proc(void* p)
{

    struct event_base *base;
    struct event *signal_event;
    struct emb_rtu_via_serial_t* rtu;

    evthread_use_pthreads();

    base = event_base_new();

    signal_event = evsignal_new(base, SIGINT, signal_cb, (void *)base);
    if (!signal_event || event_add(signal_event, NULL)<0) {
        fprintf(stderr, "Could not create/add a signal event!\n");
        fflush(stderr);
    }

    // Creatig the client
    client = emb_posix_sync_client_create(base);

    // Creating the RTU
    rtu = emb_rtu_via_serial_create(base, 10, "/dev/ttyUSB0", 115200);

    // Bind the client to the RTU
    emb_posix_sync_client_set_transport(client, emb_rtu_via_serial_get_transport(rtu));

    // Enable the packets dumping
    emb_rtu_via_serial_get_transport(rtu)->flags |= EMB_TRANSPORT_FLAG_DUMD_PAKETS;
    emb_posix_dumping_stream = stdout;
    emb_posix_dumper_enable_rx_tx();

    // Libevent's loop
    event_base_dispatch(base);

    // Destroy the RTU
    emb_rtu_via_serial_destroy(rtu);

    // Destroy the Client
    emb_posix_sync_client_destroy(client);

    exit(0);
}

// Small helpers.
int alloc_pdu_data(emb_pdu_t* _pdu)
{
    if(_pdu) {
        if((_pdu->data = malloc(MAX_PDU_DATA_SIZE))) {
            _pdu->max_size = MAX_PDU_DATA_SIZE;
            return 0;
        }
        else {
            return -ENOMEM;
        }
    }
    return -EINVAL;
}

int free_pdu_data(emb_pdu_t* _pdu)
{
    if(_pdu) {
        if(_pdu->data) {
            free(_pdu->data);
            _pdu->data = NULL;
            _pdu->max_size;
            return 0;
        }
    }
    return -EINVAL;
}

/***************************************************************************************************/
// Main

int main()
{
    int i;
    pthread_t pthr;

    pthread_create(&pthr, NULL, thr_proc, NULL);
    sleep(1);   // We must wait here for all initialization is made in started thread.

    emb_pdu_t req;
    emb_pdu_t ans;
    struct emb_client_transaction_t transaction;
    transaction.req_pdu = MB_CONST_PDU(&req);
    transaction.resp_pdu = &ans;
    transaction.procs = NULL;

    alloc_pdu_data(&req);
    alloc_pdu_data(&ans);

    emb_read_regs_make_req(&req, EMB_RR_HOLDINGS, 0x0000, 8);

    for(i=0; i<1000; ++i) {
        int res;

        /* Read holding registers 0x0000-0x0007 from the server 16 */
        res = emb_posix_sync_client_transaction(client, 16, 1000, &transaction);
        if(res)
            printf("Fail with transaction: %d, %s\n", res, emb_strerror(-res));

        res = emb_posix_sync_client_transaction(client, 17, 1000, &transaction);
        if(res)
            printf("Fail with transaction: %d, %s\n", res, emb_strerror(-res));
    }

    free_pdu_data(&req);
    free_pdu_data(&ans);

    pthread_kill(pthr, SIGTERM);
   //pthread_join(pthr, NULL);
    exit(0);
}
