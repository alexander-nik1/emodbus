
#include <stdio.h>
#include <string.h>

#include "emodbus/base/add/container_of.h"
#include "emodbus/base/modbus_errno.h"
#include "emodbus/base/bit_array.h"
#include "emodbus/server/server.h"
#include "emodbus/protocols/ascii.h"
#include "emodbus/protocols/tcp.h"
#include "emodbus/protocols/rtu.h"
#include "emodbus/base/modbus_errno.h"

#include "emodbus/impl/posix/tcp-server.h"
#include "emodbus/impl/posix/serial_port.h"

#define ARR_SIZE(_arr_)     (sizeof(_arr_)/sizeof(_arr_[0]))

// =============================================================================================
// Coils/Discrete inputs

static emb_ba_word_t coils[65536 / EMB_BA_N_WORD_BITS];

static uint8_t read_bits(struct emb_srv_bits_t* _coils,
                         uint16_t _offset,
                         uint16_t _quantity,
                         uint8_t* _pvalues)
{
    int r;
    r = emb_bit_arr_get_bits(coils, sizeof(coils), _pvalues, _coils->start + _offset, _quantity);
    return r == 0 ? 0 : MBE_ILLEGAL_DATA_ADDR;
}

static uint8_t write_bits(struct emb_srv_bits_t* _coils,
                          uint16_t _offset,
                          uint16_t _quantity,
                          const uint8_t* _pvalues)
{
    int r;
    r = emb_bit_arr_set_bits(coils, sizeof(coils), _pvalues, _coils->start + _offset, _quantity);
    return r == 0 ? 0 : MBE_ILLEGAL_DATA_ADDR;
}

static struct emb_srv_bits_t crs[4] =
{
    {
        .start = 0x0000,
        .size = 0x7FED,
        .read_bits = read_bits,
        .write_bits = write_bits
    },
    {
        .start = 0x7FED,
        .size = 0x0077,
        .read_bits = read_bits,
        .write_bits = write_bits
    },
    {
        .start = 0x8065,
        .size = 0x0001,
        .read_bits = read_bits,
        .write_bits = write_bits
    },
    {
        .start = 0xE800,
        .size = 0x1800,
        .read_bits = read_bits,
        .write_bits = write_bits
    }
};

// =============================================================================================
// Input/Holding registers

static uint16_t regs[65536];

static uint8_t holdings1_read_regs(struct emb_srv_regs_t* _rr,
                                   uint16_t _offset,
                                   uint16_t _quantity,
                                   uint16_t* _pvalues)
{
    memcpy(_pvalues, regs + _rr->start + _offset, _quantity * sizeof(uint16_t));
    return 0;
}

static uint8_t holdings1_write_regs(struct emb_srv_regs_t* _rr,
                                    uint16_t _offset,
                                    uint16_t _quantity,
                                    const uint16_t* _pvalues)
{
    memcpy(regs + _rr->start + _offset, _pvalues, _quantity * sizeof(uint16_t));
    return 0;
}

static struct emb_srv_regs_t rrs[4] =
{
    {
        .start = 0x0000,
        .size = 0x7FED,
        .read_regs = holdings1_read_regs,
        .write_regs = holdings1_write_regs
    },
    {
        .start = 0x7FED,
        .size = 0x0077,
        .read_regs = holdings1_read_regs,
        .write_regs = holdings1_write_regs
    },
    {
        .start = 0x8065,
        .size = 0x0001,
        .read_regs = holdings1_read_regs,
        .write_regs = holdings1_write_regs
    },
    {
        .start = 0xE800,
        .size = 0x1800,
        .read_regs = holdings1_read_regs,
        .write_regs = holdings1_write_regs
    }
};

// =============================================================================================
// Server part

static int is_addr_belongs_to_cr(const struct emb_srv_bits_t* _rr, uint16_t _addr)
{
    if(_rr)
        return (_rr->start <= _addr) && (_addr < (_rr->start + _rr->size));
    return -EINVAL;
}

static int is_addr_belongs_to_rr(const struct emb_srv_regs_t* _rr, uint16_t _addr)
{
    if(_rr)
        return (_rr->start <= _addr) && (_addr < (_rr->start + _rr->size));
    return -EINVAL;
}

static struct emb_srv_bits_t* get_coils(struct emb_server_t* _srv, uint16_t _begin)
{
    (void)_srv;
    size_t i;
    for(i=0; i<ARR_SIZE(rrs); ++i)
        if(is_addr_belongs_to_cr(&crs[i], _begin))
            return &crs[i];
    return NULL;
}

static struct emb_srv_bits_t* get_discrete_inputs(struct emb_server_t* _srv, uint16_t _begin)
{
    (void)_srv;
    size_t i;
    for(i=0; i<ARR_SIZE(rrs); ++i)
        if(is_addr_belongs_to_cr(&crs[i], _begin))
            return &crs[i];
    return NULL;
}

static struct emb_srv_regs_t* get_holding_regs(struct emb_server_t* _srv, uint16_t _begin)
{
    (void)_srv;
    size_t i;
    for(i=0; i<ARR_SIZE(rrs); ++i)
        if(is_addr_belongs_to_rr(&rrs[i], _begin))
            return &rrs[i];
    return NULL;
}

static struct emb_srv_regs_t* get_input_regs(struct emb_server_t* _srv, uint16_t _begin)
{
    (void)_srv;
    size_t i;
    for(i=0; i<ARR_SIZE(rrs); ++i)
        if(is_addr_belongs_to_rr(&rrs[i], _begin))
            return &rrs[i];
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

    size_t i;

    const struct emb_srv_regs_t* reg_range = NULL;

    for(i=0; i<ARR_SIZE(rrs); ++i)
        if(is_addr_belongs_to_rr(&rrs[i], _address))
            reg_range = &rrs[i];

    // just for testing the exception reaction
    if(0xABC0 <= _address && _address <= 0xABCF)
        return MBE_ILLEGAL_DATA_ADDR;

    if(!reg_range) {
        *_fifo_count = 0;
        return 0;
    }

    _address -= reg_range->start;

    size_t sz = reg_range->size - _address;

    if(sz > EMB_SRV_READ_FIFO_MAX_REGS)
        sz = EMB_SRV_READ_FIFO_MAX_REGS;

    memcpy(_fifo_buf, regs + reg_range->start + _address, sz * 2);

    *_fifo_count = (uint8_t)sz;

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

static struct emb_server_t* get_server(struct emb_super_server_t* _ssrv, uint8_t _address)
{
    (void)_ssrv;
    return _address == 1 ? &server : NULL;
}

static void on_event(struct emb_super_server_t* _ssrv, enum emb_super_server_event_t _event, uint8_t _data)
{
    (void)_ssrv;
    (void)_event;
    (void)_data;
//    printf("super server on_event: ");
//    switch(_event) {
//    case embsev_on_receive_pkt: printf("Packet was received"); break;
//    case embsev_no_srv: printf("Server was not found for request's id"); break;
//    case embsev_mb_exception: printf("Some exception was sent as response to incorrect request"); break;
//    case embsev_resp_sent: printf("Response was sent"); break;
//    case embsev_transport_error: printf("There was an error in a transport level"); break;
//    }
//    printf("\n");
}

static struct emb_super_server_t emb_super_server =
{
    .get_server = get_server,
    .on_event = on_event
};

// =============================================================================================
// TCP part

static uint8_t buf[256+16];

static uint8_t rx_buf[MAX_PDU_DATA_SIZE];
static uint8_t tx_buf[MAX_PDU_DATA_SIZE];

static emb_adu_t rx_adu = {
    .pdu = {
        .data = rx_buf,
        .max_size = MAX_PDU_DATA_SIZE
    }
};

static emb_adu_t tx_adu = {
    .pdu = {
        .data = tx_buf,
        .max_size = MAX_PDU_DATA_SIZE
    }
};

int main()
{
    int tmp;

    emb_tcp_server_t tcp_server;

    tcp_server.rx_timeout_ms = 1000;

    memset(regs, 0, sizeof(regs));

    memset(buf, 0, sizeof(buf));

    emb_super_server_init(&emb_super_server);

    if(emb_tcp_server_init(&tcp_server, htonl(INADDR_ANY), 8502)) {
        fprintf(stderr, "Error with tcp_server_init() : %m\n");
    }

    printf("Server start, listening at %s:%d\n", inet_ntoa(tcp_server.serveraddr.sin_addr), htons(tcp_server.serveraddr.sin_port));

    for (;;) {
        int client_id;

        // Receive
        tmp = emb_tcp_server_receive(&tcp_server, &client_id, buf, sizeof(buf));
        if(tmp == -ETIMEDOUT) {
            continue;
        }
        else if(tmp < 0) {
            fprintf(stderr, "Error with tcp_server_read(): %s\n", emb_strerror(-tmp));
            break;
        }
        else if(tmp == 0) {
            continue;
        }

        // Decode request
        tmp = emb_tcp_decode_packet(buf, (unsigned int)tmp, &rx_adu);
        if(tmp != 0) {
            fprintf(stderr, "Error with emb_tcp_decode_packet(): %d\n", tmp);
            continue;
        }

        // Process request
        tmp = emb_super_server_process_req(&emb_super_server, &rx_adu, &tx_adu);
        if(tmp < 0) {
            fprintf(stderr, "Error with emb_super_server_process_req() :%d\n", tmp);
            continue;
        }

        // Encode answer
        tmp = emb_tcp_encode_packet(&tx_adu, buf, sizeof(buf));
        if(tmp < 0) {
            fprintf(stderr, "Error with emb_tcp_encode_packet() :%d\n", tmp);
            continue;
        }

        // Send answer
        emb_tcp_server_send(&tcp_server, client_id, buf, (unsigned int)tmp);
        if(tmp < 0) {
            fprintf(stderr, "Error with serial_port_send(): %d\n", tmp);
        }
    }

    emb_tcp_server_deinit(&tcp_server);

    return 0;
}
