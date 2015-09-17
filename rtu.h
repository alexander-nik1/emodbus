
#ifndef MODBUS_MASTER_RTU_H
#define MODBUS_MASTER_RTU_H

#include "add/stream.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*modbus_rtu_on_packet_t)(void* _user_data, const void* _packet, unsigned int _size);
typedef void (*modbus_rtu_on_char_t)(void* _user_data);

struct modbus_rtu_t {

    unsigned char* rx_buffer;
    unsigned char* tx_buffer;
    unsigned int rx_buf_size;
    unsigned int tx_buf_size;

    unsigned int rx_buf_counter;

    unsigned int tx_buf_counter;
    unsigned int tx_pkt_size;

    void* user_data;

    // Calls when, the packet received
    modbus_rtu_on_packet_t modbus_rtu_on_packet;

    // Calls when, the symbol received
    modbus_rtu_on_char_t modbus_rtu_on_char;

    struct input_stream_t input_stream;
    struct output_stream_t output_stream;
};

// Call this function before anything else.
void modbus_rtu_initialize(struct modbus_rtu_t* _mbt);

// User should call this function, when time of last received symbol is expired.
void modbus_rtu_on_symbol_timeout(struct modbus_rtu_t* _mbt);

// This functions will send packet.
int modbus_rtu_send_packet(struct modbus_rtu_t* _mbt, const void* _pkt, unsigned int _size);
int modbus_rtu_send_packet_sync(struct modbus_rtu_t* _mbt, const void* _pkt, unsigned int _size);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_MASTER_RTU_H
