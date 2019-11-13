
#ifndef EMODBUS_SERVER_BASE_H
#define EMODBUS_SERVER_BASE_H

#include <emodbus/base/modbus_adu.h>
#include <emodbus/server/bits.h>
#include <emodbus/server/regs.h>
#include <emodbus/server/file.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum emb_super_server_event_t {
    embsev_on_receive_pkt,
    embsev_no_srv,
    embsev_mb_exception,
    embsev_resp_sent,
    embsev_transport_error
};

struct emb_server_t;
struct emb_super_server_t;

typedef uint8_t (*emb_srv_function_t)(struct emb_super_server_t* _ssrv, struct emb_server_t* _srv);

enum {
    EMB_SRV_BROADCAST_FLAG = (1 << 0)
};

struct emb_server_t {

    emb_srv_function_t (*get_function)(struct emb_server_t* _srv, uint8_t _func);

    struct emb_srv_bits_t* (*get_coils)(struct emb_server_t* _srv, uint16_t _begin);

    struct emb_srv_bits_t* (*get_discrete_inputs)(struct emb_server_t* _srv, uint16_t _begin);

    struct emb_srv_regs_t* (*get_holding_regs)(struct emb_server_t* _srv, uint16_t _begin);

    struct emb_srv_regs_t* (*get_input_regs)(struct emb_server_t* _srv, uint16_t _begin);

    struct emb_srv_file_t* (*get_file)(struct emb_server_t* _srv, uint16_t _fileno/*, uint16_t _begin*/);

    uint8_t (*read_fifo)(struct emb_server_t* _srv, uint16_t _address,
                         uint16_t* _fifo_buf, uint8_t* _fifo_count);

    uint8_t flags;
};

struct emb_super_server_t {

    struct emb_server_t* (*get_server)(struct emb_super_server_t* _ssrv,
                                       uint8_t _address);

    void (*on_event)(struct emb_super_server_t* _ssrv, enum emb_super_server_event_t _event, uint8_t data);

    emb_pdu_t* rx_pdu;
    emb_pdu_t* tx_pdu;
};

void emb_super_server_init(struct emb_super_server_t* _ssrv);

int emb_build_exception_pdu(emb_pdu_t* _result,
                            uint8_t _func,
                            uint8_t _errno);

int emb_super_server_process_req(struct emb_super_server_t* _ssrv,
                                 const  emb_adu_t* _rx_adu,
                                 emb_adu_t* _tx_adu);

//**********************************************************************
// Coils and Discrete inputs.

uint8_t emb_srv_read_bits(struct emb_super_server_t* _ssrv,
                          struct emb_server_t* _srv);

uint8_t emb_srv_write_coil(struct emb_super_server_t* _ssrv,
                           struct emb_server_t* _srv);

uint8_t emb_srv_write_coils(struct emb_super_server_t* _ssrv,
                            struct emb_server_t* _srv);

//**********************************************************************
// Holding and Input registers

uint8_t emb_srv_read_regs(struct emb_super_server_t* _ssrv,
                          struct emb_server_t* _srv);

uint8_t emb_srv_write_reg(struct emb_super_server_t* _ssrv,
                          struct emb_server_t* _srv);

uint8_t emb_srv_write_regs(struct emb_super_server_t* _ssrv,
                           struct emb_server_t* _srv);

uint8_t emb_srv_mask_reg(struct emb_super_server_t* _ssrv,
                         struct emb_server_t* _srv);


enum { EMB_SRV_RDWR_REGS_MAX_READ_REGS = 0x007D };
enum { EMB_SRV_RDWR_REGS_MAX_WRITE_REGS = 0x0079 };

uint8_t emb_srv_read_write_regs(struct emb_super_server_t* _ssrv,
                                struct emb_server_t* _srv);

//**********************************************************************
// File records

uint8_t emb_srv_read_file(struct emb_super_server_t* _ssrv,
                          struct emb_server_t* _srv);

uint8_t emb_srv_write_file(struct emb_super_server_t* _ssrv,
                           struct emb_server_t* _srv);

//**********************************************************************
// FIFOs

enum { EMB_SRV_READ_FIFO_MAX_REGS = 31 };

uint8_t emb_srv_read_fifo(struct emb_super_server_t* _ssrv,
                          struct emb_server_t* _srv);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // EMODBUS_SERVER_BASE_H

