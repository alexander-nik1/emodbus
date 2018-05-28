

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include <emodbus/server/server.h>
#include <emodbus/base/modbus_pdu.h>
#include <emodbus/transport/rtu.h>
#include <emodbus/base/modbus_errno.h>
#include <emodbus/base/add/container_of.h>
#include <emodbus/base/common.h>
#include "rtu_via_tty.h"

static int is_addr_belongs_to_holdings(uint16_t _addr, const struct emb_srv_regs_t* _holdings)
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
    (void)_rr;
    memcpy(_pvalues, holdings1_regs + _offset, _quantity);
    return 0;
}

uint8_t holdings1_write_regs(struct emb_srv_regs_t* _rr,
                      uint16_t _offset,
                      uint16_t _quantity,
                      const uint16_t* _pvalues)
{
    (void)_rr;
    memcpy(holdings1_regs + _offset, _pvalues, _quantity);
    return 0;
}

struct emb_srv_regs_t holdings1 = {
    .start = 0x1000,
    .size = sizeof(holdings1_regs)/sizeof(uint16_t),
    .read_regs = holdings1_read_regs,
    .write_regs = holdings1_write_regs
};

// ************************************************************************************************************************
// modbus server

// Functions, that will be supported by this server.
static emb_srv_function_t my_srv_funcs[] = {
    /* 0x00 */ NULL,
    /* 0x01 */ emb_srv_read_bits,
    /* 0x02 */ emb_srv_read_bits,
    /* 0x03 */ emb_srv_read_regs,
    /* 0x04 */ emb_srv_read_regs,
    /* 0x05 */ emb_srv_write_coil,
    /* 0x06 */ emb_srv_write_reg,
    /* 0x07 */ NULL,
    /* 0x08 */ NULL,
    /* 0x09 */ NULL,
    /* 0x0A */ NULL,
    /* 0x0B */ NULL,
    /* 0x0C */ NULL,
    /* 0x0D */ NULL,
    /* 0x0E */ NULL,
    /* 0x0F */ emb_srv_write_coils,
    /* 0x10 */ emb_srv_write_regs,
    /* 0x11 */ NULL,
    /* 0x12 */ NULL,
    /* 0x13 */ NULL,
    /* 0x14 */ emb_srv_read_file,
    /* 0x15 */ emb_srv_write_file,
    /* 0x16 */ emb_srv_mask_reg,
    /* 0x17 */ emb_srv_read_write_regs,
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

/********************************************************************************/

struct rtu_via_tty_t rtu_via_tty;

// *******************************************************************************

#if EMODBUS_PACKETS_DUMPING
void dump_packet(const void* _data, unsigned int _size)
{
    unsigned int i;
    for(i=0; i<_size; i++) {
        printf(" %02X", ((uint8_t*)_data)[i]);
    }
    printf("\n");
}

void dump_rx_packet(const void* _data, unsigned int _size)
{
    printf(">>");
    dump_packet(_data, _size);
}

void dump_tx_packet(const void* _data, unsigned int _size)
{
    printf("<<");
    dump_packet(_data, _size);
}
#endif

int main(int argc, char* argv[])
{
    memset(holdings1_regs, 0, sizeof(holdings1_regs));

    emb_super_server_init(&mb_ssrv);

    rtu_via_tty_init(&rtu_via_tty, argv[1], atoi(argv[2]));

    rtu_via_tty.rtu.transport.flags |= EMB_TRANSPORT_FLAG_DUMD_PAKETS;

    emb_super_server_set_transport(&mb_ssrv, &rtu_via_tty.rtu.transport);

#if EMODBUS_PACKETS_DUMPING
    emb_dump_rx_data = dump_rx_packet;
    emb_dump_tx_data = dump_tx_packet;
#endif

    if(rtu_via_tty_open(&rtu_via_tty) == 0) {

        while(1) {
            int res = rtu_via_tty_receive_pdu(&rtu_via_tty, 10000);
            if(res != 0) {
                printf("Receive error: %s\n", emb_strerror(-res));
            }
            else {

                if(emb_rtu_has_data_to_send(&rtu_via_tty.rtu)) {
                    res = rtu_via_tty_send_pdu(&rtu_via_tty, 10000);
                    if(res != 0) {
                        printf("Send error: %s\n", emb_strerror(-res));
                    }
                }
            }
        }
        rtu_via_tty_close(&rtu_via_tty);
    }

    return 0;
}
