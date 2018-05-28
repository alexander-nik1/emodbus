
#ifndef EMB_EXAMPLE_RTU_VIA_TTY_H
#define EMB_EXAMPLE_RTU_VIA_TTY_H

#include <emodbus/transport/rtu.h>

struct rtu_via_tty_t
{
    struct emb_rtu_t rtu;
    int fd;
    const char* tty_name;
    unsigned baudrate;
    int is_opened;

    char rx_buf[MAX_PDU_SIZE];
    char tx_buf[MAX_PDU_SIZE];
};

void rtu_via_tty_init(struct rtu_via_tty_t* _ctx,
                     const char* _tty_name,
                     unsigned int _baudrate);

int rtu_via_tty_open(struct rtu_via_tty_t* _ctx);

void rtu_via_tty_close(struct rtu_via_tty_t* _ctx);

int rtu_via_tty_set_baudrate(struct rtu_via_tty_t* _ctx,
                             unsigned int _baudrate);

int rtu_via_tty_receive_pdu(struct rtu_via_tty_t* _ctx,
                            int _timeout_msec);

int rtu_via_tty_send_pdu(struct rtu_via_tty_t* _ctx,
                         int _timeout_msec);

#endif // EMB_EXAMPLE_RTU_VIA_TTY_H
