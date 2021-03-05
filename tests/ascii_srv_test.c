
#include <stdio.h>
#include <string.h>


#include "emodbus/base/modbus_errno.h"
#include "emodbus/server/server.h"
#include "emodbus/protocols/ascii.h"
#include "emodbus/impl/posix/serial_port.h"

#define ARR_SIZE(_arr_)     (sizeof(_arr_)/sizeof(_arr_[0]))

static int is_addr_belongs_to_holdings(uint16_t _addr, const struct emb_srv_regs_t* _holdings)
{
    return ((_holdings->start <= _addr) && (_addr < _holdings->start + _holdings->size));
}

// =============================================================================================
// Holding registers: 0x1000-0x100F

static uint16_t holdings1_regs[0x10000];

uint8_t holdings1_read_regs(struct emb_srv_regs_t* _rr,
                            uint16_t _offset,
                            uint16_t _quantity,
                            uint16_t* _pvalues)
{
    (void)_rr;
//    printf("===read before:  0x%04X\n", holdings1_regs[0]);
    memcpy(_pvalues, holdings1_regs + _offset, _quantity * 2);
//    printf("===read after:   0x%04X\n", holdings1_regs[0]);
    return 0;
}

uint8_t holdings1_write_regs(struct emb_srv_regs_t* _rr,
                      uint16_t _offset,
                      uint16_t _quantity,
                      const uint16_t* _pvalues)
{
    (void)_rr;
//    printf("===write before: 0x%04X\n", holdings1_regs[0]);
    memcpy(holdings1_regs + _offset, _pvalues, _quantity * 2);
//    printf("===write after:  0x%04X\n", holdings1_regs[0]);
    return 0;
}

static struct emb_srv_regs_t holdings1 =
{
    .start = 0,
    .size = sizeof(holdings1_regs)/sizeof(uint16_t),
    .read_regs = holdings1_read_regs,
    .write_regs = holdings1_write_regs
};

// =============================================================================================
// Server part

static struct emb_srv_bits_t* get_coils(struct emb_server_t* _srv, uint16_t _begin)
{
    (void)_srv;
    (void)_begin;
    return NULL;
}

static struct emb_srv_bits_t* get_discrete_inputs(struct emb_server_t* _srv, uint16_t _begin)
{
    (void)_srv;
    (void)_begin;
    return NULL;
}

static struct emb_srv_regs_t* get_holding_regs(struct emb_server_t* _srv, uint16_t _begin)
{
    (void)_srv;
    if(is_addr_belongs_to_holdings(_begin, &holdings1))
        return &holdings1;
    return NULL;
}

static struct emb_srv_regs_t* get_input_regs(struct emb_server_t* _srv, uint16_t _begin)
{
    (void)_srv;
    (void)_begin;
    return NULL;
}

static struct emb_srv_file_t* get_file(struct emb_server_t* _srv, uint16_t _fileno/*, uint16_t _begin*/)
{
    (void)_srv;
    (void)_fileno;
    return NULL;
}

static uint8_t read_fifo(struct emb_server_t* _srv, uint16_t _address,
                         uint16_t* _fifo_buf, uint8_t* _fifo_count)
{
    (void)_srv;
    (void)_address;
    (void)_fifo_buf;
    (void)_fifo_count;
    return 0;
}

static const emb_srv_function_t functions[] =
{
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
    /* 0x18 */ emb_srv_read_fifo
};

static emb_srv_function_t get_function(struct emb_server_t* _srv, uint8_t _func)
{
    (void)_srv;
    if(_func < ARR_SIZE(functions))
        return functions[_func];
    else
        return NULL;
}

static struct emb_server_t server =
{
    .get_function = get_function,
    .get_coils = get_coils,
    .get_discrete_inputs = get_discrete_inputs,
    .get_holding_regs = get_holding_regs,
    .get_input_regs = get_input_regs,
    .get_file = get_file,
    .read_fifo = read_fifo,
    .flags = 0
};

// =============================================================================================
// Super server part

struct emb_server_t* get_server(struct emb_super_server_t* _ssrv, uint8_t _address)
{
    (void)_ssrv;
    return _address == 1 ? &server : NULL;
}

//void on_event(struct emb_super_server_t* _ssrv, enum emb_super_server_event_t _event, uint8_t _data)
//{
//    (void)_ssrv;
//    (void)_event;
//    (void)_data;
//    printf("super server on_event: ");
//    switch(_event) {
//    case embsev_on_receive_pkt: printf("Packet was received"); break;
//    case embsev_no_srv: printf("Server was not found for request's id"); break;
//    case embsev_mb_exception: printf("Some exception was sent as response to incorrect request"); break;
//    case embsev_resp_sent: printf("Response was sent"); break;
//    case embsev_transport_error: printf("There was an error in a transport level"); break;
//    }
//    printf("\n");
//}

static struct emb_super_server_t emb_super_server =
{
    .get_server = get_server
//    .on_event = on_event
};

// =============================================================================================
// RTU part

static struct emb_serial_port_t serial_port =
{
    .tty_name = "/dev/ttyUSB1",
    .baudrate = 115200,
    .final_delay_ms = 100
};

void print_adu(FILE* _f, const emb_adu_t* _adu)
{
    uint8_t i;
    fprintf(_f, "srv:0x%02X f:0x%02X data:", _adu->server_id, _adu->pdu.function);
    for(i=0; i<_adu->pdu.data_size; ++i) {
        fprintf(_f, "%02X ", ((uint8_t*)_adu->pdu.data)[i]);
    }
    fprintf(_f, "\n");
}

enum { RX_TIMEOUT = 100 };
enum { TX_TIMEOUT = 100 };

int main()
{
    uint8_t buf[256 * 2 + 16];

    uint8_t rx_buf[MAX_PDU_DATA_SIZE];
    uint8_t tx_buf[MAX_PDU_DATA_SIZE];

    emb_adu_t rx_adu = {
        .pdu = {
            .data = rx_buf,
            .max_size = MAX_PDU_DATA_SIZE
        }
    };

    emb_adu_t tx_adu = {
        .pdu = {
            .data = tx_buf,
            .max_size = MAX_PDU_DATA_SIZE
        }
    };

    memset(holdings1_regs, 0, sizeof(holdings1_regs));

    memset(buf, 0, sizeof(buf));

    emb_super_server_init(&emb_super_server);

    emb_serial_port_init(&serial_port);
    if(emb_serial_port_open(&serial_port) != 0) {
        fprintf(stderr, "Error: serial_port_open() : %m\n");
        return -1;
    }

    while(1) {
        memset(buf, 0, sizeof(buf));
        memset(rx_buf, 0, sizeof(rx_buf));

        int tmp;
        tmp = emb_serial_port_receive_ascii(&serial_port, buf, sizeof(buf), RX_TIMEOUT);
        if(tmp < 0) {
            if(tmp != -modbus_timeout)
                fprintf(stderr, "Error with serial_port_receive(): %s\n", emb_strerror(-tmp));
            continue;
        }

        tmp = emb_ascii_decode_packet(buf, (unsigned int)tmp, &rx_adu);
        if(tmp != 0) {
            fprintf(stderr, "Error with emb_rtu_decode_packet(): %d\n", tmp);
            continue;
        }

//        printf(">> ");
//        print_adu(stdout, &rx_adu);

        tmp = emb_super_server_process_req(&emb_super_server, &rx_adu, &tx_adu);
        if(tmp < 0) {
            fprintf(stderr, "Error with emb_super_server_process_req() :%d\n", tmp);
            continue;
        }

        tmp = emb_ascii_encode_packet(&tx_adu, buf, sizeof(buf));
        if(tmp < 0) {
            fprintf(stderr, "Error with emb_rtu_encode_packet() :%d\n", tmp);
            continue;
        }
        else if(tmp > 0) {
//            printf("<< ");
//            print_adu(stdout, &tx_adu);
            tmp = emb_serial_port_send(&serial_port, buf, (unsigned int)tmp, TX_TIMEOUT);
            if(tmp < 0) {
                fprintf(stderr, "Error with serial_port_send(): %d\n", tmp);
            }
        }
    }
}
