
#ifndef EMB_SERIAL_PORT_H
#define EMB_SERIAL_PORT_H

struct emb_serial_port_t
{
    const char* tty_name;
    unsigned baudrate;
    unsigned long timeout_ms;
    unsigned long override_final_delay_ms;

    int fd;
    unsigned long rx_bytes_counter;
    unsigned long tx_bytes_counter;
    unsigned long tx_packets;
    unsigned long rx_packets;
};

void emb_serial_port_init(struct emb_serial_port_t* _ctx);

int emb_serial_port_open(struct emb_serial_port_t* _ctx);

void emb_serial_port_close(struct emb_serial_port_t* _ctx);

int emb_serial_port_set_baudrate(struct emb_serial_port_t* _ctx,
                             unsigned int _baudrate);

int emb_serial_port_receive(struct emb_serial_port_t* _ctx, void* _p_buffer, unsigned int _max_size);

int emb_serial_port_send(struct emb_serial_port_t* _ctx, const void* _p_data, unsigned int _size);

#endif // EMB_EXAMPLE_RTU_VIA_TTY_H
