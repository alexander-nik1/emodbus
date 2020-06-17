
#ifndef EMB_SERIAL_PORT_H
#define EMB_SERIAL_PORT_H

struct serial_port_t
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

void serial_port_init(struct serial_port_t* _ctx);

int serial_port_open(struct serial_port_t* _ctx);

void serial_port_close(struct serial_port_t* _ctx);

int serial_port_set_baudrate(struct serial_port_t* _ctx,
                             unsigned int _baudrate);

int serial_port_receive(struct serial_port_t* _ctx, void* _p_buffer, unsigned int _max_size);

int serial_port_send(struct serial_port_t* _ctx, const void* _p_data, unsigned int _size);

#endif // EMB_EXAMPLE_RTU_VIA_TTY_H
