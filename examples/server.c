
#include <stdio.h>
#include <stdlib.h>
#include <event2/event.h>
#include <event2/bufferevent_ssl.h>
#include <string.h>
#include <signal.h>

#include <emodbus/impl/posix/mb-tcp-via-tcp-server.h>
#include <emodbus/server/server.h>
#include <emodbus/base/modbus_pdu.h>

int is_addr_belongs_to_holdings(uint16_t _addr, const struct emb_srv_regs_t* _holdings)
{
    return ((_holdings->start <= _addr) && (_addr < _holdings->start + _holdings->size));
}

/***************************************************************************************************/
/*                                                                                                 */
/*                                         SERVER EXAMPLE                                          */
/*                                                                                                 */
/***************************************************************************************************/

/***************************************************************************************************/
// Holding registers: 0x1000-0x100F

uint16_t holdings1_regs[0x10];

uint8_t holdings1_read_regs(struct emb_srv_regs_t* _rr,
                            uint16_t _offset,
                            uint16_t _quantity,
                            uint16_t* _pvalues)
{
    memcpy(_pvalues, holdings1_regs + _offset, _quantity);
    return 0;
}

uint8_t holdings1_write_regs(struct emb_srv_regs_t* _rr,
                      uint16_t _offset,
                      uint16_t _quantity,
                      const uint16_t* _pvalues)
{
    memcpy(holdings1_regs + _offset, _pvalues, _quantity);
    return 0;
}

struct emb_srv_regs_t holdings1 = {
    .start = 0x1000,
    .size = sizeof(holdings1_regs)/sizeof(uint16_t),
    .read_regs = holdings1_read_regs,
    .write_regs = holdings1_write_regs
};

/***************************************************************************************************/
// FIFO

static const uint16_t fifo1_data[8] =
{
    0xDEAD, 0x1111, 0x2222, 0x3333, 0x4444, 0x5555, 0x6666, 0x7777
};

static uint8_t srv_read_fifo(struct emb_server_t* _srv, uint16_t _address,
                      uint16_t* _fifo_buf, uint8_t* _fifo_count)
{
    if(_address < 8) {
        memcpy(_fifo_buf, fifo1_data+_address, (8-_address)*2);
        *_fifo_count = 8-_address;
    }
    else {
        *_fifo_count = 0;
    }
    return 0;
}

// ************************************************************************************************************************
// modbus server

// Functions, that will be supported by this server.
static emb_srv_function_t my_srv_funcs[] = {
    /* 0x00 */ NULL,
    /* 0x01 */ NULL, // emb_srv_read_coils,
    /* 0x02 */ NULL,
    /* 0x03 */ emb_srv_read_regs,
    /* 0x04 */ NULL,
    /* 0x05 */ NULL, // emb_srv_write_coil,
    /* 0x06 */ emb_srv_write_reg,
    /* 0x07 */ NULL,
    /* 0x08 */ NULL,
    /* 0x09 */ NULL,
    /* 0x0A */ NULL,
    /* 0x0B */ NULL,
    /* 0x0C */ NULL,
    /* 0x0D */ NULL,
    /* 0x0E */ NULL,
    /* 0x0F */ NULL, // emb_srv_write_coils,
    /* 0x10 */ emb_srv_write_regs,
    /* 0x11 */ NULL,
    /* 0x12 */ NULL,
    /* 0x13 */ NULL,
    /* 0x14 */ NULL, // emb_srv_read_file,
    /* 0x15 */ NULL, // emb_srv_write_file,
    /* 0x16 */ emb_srv_mask_reg,
    /* 0x17 */ NULL,
    /* 0x18 */ emb_srv_read_fifo,
};

static emb_srv_function_t my_srv_get_function(struct emb_server_t* _srv, uint8_t _func)
{
    if(_func < (sizeof(my_srv_funcs)/sizeof(emb_srv_function_t)))
        return my_srv_funcs[_func];
    else
        return NULL;
}

static struct emb_srv_regs_t* my_srv_get_holdings(struct emb_server_t* _srv, uint16_t _begin)
{
    if(is_addr_belongs_to_holdings(_begin, &holdings1)) {
        return &holdings1;
    }
    return NULL;
}

struct emb_server_t my_srv = {
    .get_function = my_srv_get_function,
    .get_coils = NULL,
    .get_holding_regs = my_srv_get_holdings,
    .get_file = NULL,
    .read_fifo = srv_read_fifo
};

// ************************************************************************************************************************
// modbus super server

static struct emb_server_t* mb_ssrv_get_server(struct emb_super_server_t* _ssrv, uint8_t _address)
{
    if(_address == 0x10)
        return &my_srv;
    return NULL;
}

static uint8_t rx_buf[MAX_PDU_DATA_SIZE];
static uint8_t tx_buf[MAX_PDU_DATA_SIZE];

static emb_pdu_t rx_pdu = {
    .data = rx_buf,
    .max_size = MAX_PDU_DATA_SIZE
};

static emb_pdu_t tx_pdu = {
    .data = tx_buf,
    .max_size = MAX_PDU_DATA_SIZE
};

static struct emb_super_server_t mb_ssrv = {
    .get_server = mb_ssrv_get_server,
    .rx_pdu = &rx_pdu,
    .tx_pdu = &tx_pdu,
    .on_event = NULL
};

// ************************************************************************************************************************

static void signal_cb(evutil_socket_t sig, short events, void *user_data)
{
    struct event_base *base = user_data;
    struct timeval delay = { 1, 0 };
    printf("Caught an interrupt signal; exiting cleanly in one second.\n");
    event_base_loopexit(base, &delay);
}

enum { TCP_LISTEN_PORT = 8502 };

int main(int argc, char* argv[]) {

    struct event_base *base = NULL;

    struct event *signal_event;

    struct tcp_server_t* tcp_srv;

    struct emb_tcp_via_tcp_server_t* mb_tcp;

    printf("EModbus TCP server example, listening port: %d\n", TCP_LISTEN_PORT);

    memset(holdings1_regs, 0, sizeof(holdings1_regs));

    base = event_base_new();
    if(!base) {
        fprintf(stderr, "Error: event_base_new() return NULL : %m\n");
        fflush(stderr);
        goto ret;
    }

    signal_event = evsignal_new(base, SIGINT, signal_cb, (void *)base);
    if (!signal_event || event_add(signal_event, NULL)<0) {
        fprintf(stderr, "Could not create/add a signal event!\n");
        fflush(stderr);
        goto ret;
    }

    mb_tcp = emb_tcp_via_tcp_server_create(base, TCP_LISTEN_PORT, 1000);
    if(!mb_tcp) {
        fprintf(stderr, "Error: emb_tcp_via_tcp_server_create() (port:%d) return NULL : %m\n", TCP_LISTEN_PORT);
        fflush(stderr);
        goto ret;
    }

    tcp_srv = emb_tcp_via_tcp_server_get_srv(mb_tcp);
    tcp_server_set_clients_limit(tcp_srv, 1);

    emb_super_server_init(&mb_ssrv);
    emb_super_server_set_transport(&mb_ssrv, emb_tcp_via_tcp_server_get_transport(mb_tcp));

    emb_tcp_via_tcp_server_get_transport(mb_tcp)->flags |= EMB_TRANSPORT_FLAG_DUMD_PAKETS;

    event_base_dispatch(base);

ret:
    if(signal_event)
        event_free(signal_event);
    if(mb_tcp)
        emb_tcp_via_tcp_server_destroy(mb_tcp);
    if(base)
        event_base_free(base);

    return 0;
}
