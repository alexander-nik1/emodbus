
#ifndef EMB_SERIAL_PORT_H
#define EMB_SERIAL_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    const char* tty_name;
    unsigned baudrate;
	unsigned long timeout_ms;
	unsigned long final_delay_ms;

    int fd;
    unsigned long rx_bytes_counter;
    unsigned long tx_bytes_counter;
    unsigned long tx_packets;
    unsigned long rx_packets;
} emb_serial_port_t;

void emb_serial_port_init(emb_serial_port_t* _ctx);

int emb_serial_port_open(emb_serial_port_t* _ctx);

void emb_serial_port_close(emb_serial_port_t* _ctx);

int emb_serial_port_set_baudrate(emb_serial_port_t* _ctx,
								 unsigned int _baudrate);

int emb_serial_port_receive_rtu(emb_serial_port_t* _ctx, void* _p_buffer, unsigned int _max_size);

int emb_serial_port_receive_ascii(emb_serial_port_t* _ctx, void* _p_buffer, unsigned int _max_size);

int emb_serial_port_send(emb_serial_port_t* _ctx, const void* _p_data, unsigned int _size);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // EMB_EXAMPLE_RTU_VIA_TTY_H
