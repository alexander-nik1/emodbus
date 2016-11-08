
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

/***************************************************************************************************/
/*                                                                                                 */
/*                                SYNCHRONOUS CLIENT EXAMPLE                                       */
/*                                                                                                 */
/***************************************************************************************************/

/***************************************************************************************************/
// Synchronous client

// Emodbus client context
struct emb_client_t client;
// Storage for transaction result
int transact_result = 0;
// Mutex to waiting an events on it
pthread_mutex_t mutex;
// Response timeout timer.
struct event* timeout_timer;

// This function calls when response waiting timeout occurs.
static void timeout_cb(evutil_socket_t _fd, short _events, void * _arg) {
    emb_client_wait_timeout(&client);
    transact_result=-modbus_resp_timeout;
    pthread_mutex_unlock(&mutex);
}

// This function calls when a valid response was got from server
void client_on_response(struct emb_client_t* _req, int _slave_addr) {
    transact_result=0;
    pthread_mutex_unlock(&mutex);
}

// This function calls when an error occurs, or when a modbus-error code was returned by a server.
void client_on_error(struct emb_client_t* _req, int _slave_addr, int _errno) {
    transact_result=_errno;
    pthread_mutex_unlock(&mutex);
}

// Call this function to begin the transaction.
int client_sync_transaction(int _server_addr,
                            unsigned int _timeout,
                            struct emb_client_transaction_t* _transact) {
    int res;
    struct timeval timeout_time;

    timeout_time.tv_sec = 0;
    timeout_time.tv_usec = _timeout * 1000;
    if((res = emb_client_do_transaction(&client, _server_addr, _transact)) != 0)
        return res;
    event_add(timeout_timer, &timeout_time);

    pthread_mutex_lock(&mutex);
    return transact_result;
}

/***************************************************************************************************/
// This function are called at Ctrl+C

static void signal_cb(evutil_socket_t sig, short events, void *user_data) {
    struct event_base *base = user_data;
    printf("Caught an interrupt signal; exiting cleanly.\n");
    event_base_loopexit(base, NULL);
}

/***************************************************************************************************/
// Thread for a transport-level operations.
// This thread works with a physical port.
void* thr_proc(void* p) {

    int res=0;
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

    memset(&client, 0, sizeof(struct emb_client_t));
    client.on_response = client_on_response;
    client.on_error = client_on_error;
    emb_client_init(&client);

    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_trylock(&mutex);

    timeout_timer = event_new(base, -1, EV_TIMEOUT, timeout_cb, &client);

    rtu = emb_rtu_via_serial_create(base, 10, "/dev/ttyUSB0", 115200);

    emb_client_set_transport(&client, emb_rtu_via_serial_get_transport(rtu));
    emb_rtu_via_serial_get_transport(rtu)->flags |= EMB_TRANSPORT_FLAG_DUMD_PAKETS;

    event_base_dispatch(base);

    emb_rtu_via_serial_destroy(rtu);

    if(timeout_timer)
        event_free(timeout_timer);
    exit(0);
}

// Small helpers.
int alloc_pdu_data(emb_pdu_t* _pdu) {
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

int free_pdu_data(emb_pdu_t* _pdu) {
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
// Dumping RX/TX packets

void dump_packet(void *_f, const char* _prefix, const void* _pkt, unsigned int _size) {
    if(_f) {
        FILE* f = (FILE*)_f;
        int i;
        fprintf(f, "%s", _prefix);
        for(i=0; i<_size; ++i)
            fprintf(f, "%02X ", ((uint8_t*)_pkt)[i]);
        fprintf(f, "\n");
        fflush(f);
    }
}

void on_emodbus_transport_rx(const void* _data, unsigned int _size)
{ dump_packet(stdout, ">>", _data, _size); }

void on_emodbus_transport_tx(const void* _data, unsigned int _size)
{ dump_packet(stdout, "<<", _data, _size); }

void dumping_init() {
    emb_dump_rx_data = on_emodbus_transport_rx;
    emb_dump_tx_data = on_emodbus_transport_tx;
}

/***************************************************************************************************/
// Main

int main() {
    int i;
    pthread_t pthr;

    dumping_init();

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
        res = client_sync_transaction(16, 1000, &transaction);
        if(res)
            printf("Fail with transaction: %d, %s\n", res, emb_strerror(-res));
    }

    free_pdu_data(&req);
    free_pdu_data(&ans);

    pthread_kill(pthr, SIGTERM);
   //pthread_join(pthr, NULL);
    exit(0);
}
