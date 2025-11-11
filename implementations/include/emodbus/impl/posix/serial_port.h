
#ifndef EMB_SERIAL_PORT_H
#define EMB_SERIAL_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    emb_serial_parity_disabled,
    emb_serial_parity_odd,
    emb_serial_parity_even
} emb_serial_parity_t;

typedef enum
{
    emb_serial_db5,
    emb_serial_db6,
    emb_serial_db7,
    emb_serial_db8
} emb_serial_databits_t;

typedef enum
{
    emb_serial_sb1,
    emb_serial_sb2
} emb_serial_stopbits_t;

typedef struct
{
    const char* tty_name;
    unsigned baudrate;
    emb_serial_parity_t parity;
    emb_serial_databits_t databits;
    emb_serial_stopbits_t stop_bits;
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
