
#ifndef MODBUS_MASTER_RTU_H
#define MODBUS_MASTER_RTU_H

#include "add/stream.h"

#include "modbus_proto.h"

#ifdef __cplusplus
extern "C" {
#endif

//typedef void (*modbus_rtu_on_packet_t)(void* _user_data, const void* _packet, unsigned int _size);
typedef void (*modbus_rtu_on_char_t)(void* _user_data);

struct modbus_rtu_t {

    unsigned char* rx_buffer;
    unsigned char* tx_buffer;
    unsigned int rx_buf_size;
    unsigned int tx_buf_size;

    unsigned int rx_buf_counter;

    unsigned int tx_buf_counter;
    unsigned int tx_pkt_size;

    struct modbus_pdu_t rx_pdu;
    struct modbus_pdu_t tx_pdu;

    void* user_data;

    struct modbus_protocol_t proto;

    // modbus_rtu calls this function, when the char received
    modbus_rtu_on_char_t modbus_rtu_on_char;

    struct input_stream_t input_stream;
    struct output_stream_t output_stream;
};

// Call this function before anything else.
void modbus_rtu_initialize(struct modbus_rtu_t* _mbt);

// User should call this function, when time of last received char is expired.
void modbus_rtu_on_char_timeout(struct modbus_rtu_t* _mbt);

// Send error to high level
void modbus_rtu_on_error(struct modbus_rtu_t* _mbt,
                         int _errno);

// Same as above, but waits for all data has sent.
int modbus_rtu_send_packet_sync(struct modbus_rtu_t* _mbt,
                                int _slave_addr,
                                const struct modbus_const_pdu_t* _pdu);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_MASTER_RTU_H
