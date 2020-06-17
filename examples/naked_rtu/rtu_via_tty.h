
#ifndef EMB_EXAMPLE_RTU_VIA_TTY_H
#define EMB_EXAMPLE_RTU_VIA_TTY_H

#include <emodbus/transport/rtu.h>

struct serial_port_t
{
    struct emb_rtu_t rtu;
    int fd;
    const char* tty_name;
    unsigned baudrate;
    int is_opened;

    uint8_t rx_buf[MAX_PDU_SIZE];
    uint8_t tx_buf[MAX_PDU_SIZE];
    unsigned int rx_counter;
};

void rtu_via_tty_init(struct serial_port_t* _ctx,
                     const char* _tty_name,
                     unsigned int _baudrate);

int rtu_via_tty_open(struct serial_port_t* _ctx);

void rtu_via_tty_close(struct serial_port_t* _ctx);

int rtu_via_tty_set_baudrate(struct serial_port_t* _ctx,
                             unsigned int _baudrate);

int rtu_via_tty_receive_pdu(struct serial_port_t* _ctx,
                            int _timeout_msec);

int rtu_via_tty_send_pdu(struct serial_port_t* _ctx,
                         int _timeout_msec);

#endif // EMB_EXAMPLE_RTU_VIA_TTY_H
