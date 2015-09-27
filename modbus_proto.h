
#ifndef MODBUS_MASTER_PROTOCOL_H
#define MODBUS_MASTER_PROTOCOL_H

#include "modbus_pdu.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
   ____________
  |            |
  | high level |
  | of modbus  |
  |____________|
      |    ^
      |    |
   ___V____|___
  |            |
  |   proto    | <-- this level
  |____________|
      |    ^
      |    |
   ___V____|__
  |           |
  | low level |
  | RTU,TCP,  |
  | and so on |
  |___________|

*/



typedef int (*modbus_proto_send_packet_t)(void* _user_data,
                                          int _slave_addr,
                                          const struct modbus_const_pdu_t* _pkt);

typedef void (*modbus_proto_recv_packet_t)(void* _user_data,
                                           int _slave_addr,
                                           const struct modbus_const_pdu_t* _pkt);

typedef void (*modbus_proto_error_t)(void* _user_data, int _errno);

// Interface of modbus protocol
struct modbus_protocol_t {

    void* high_level_context;
    void* low_level_context;

    // This function calls from high to low level.
    // This function calls for send a PDU via this protocol.
    modbus_proto_send_packet_t send_packet;

    // This function calls from low to high level.
    // This function calls by low level when, a new PDU has
    // received via this protocol.
    modbus_proto_recv_packet_t recv_packet;

    // This function calls from low to high level.
    // This function tells about errors.
    modbus_proto_error_t error;

    // This PDU already have a buffer (buffer inside protocol)
    // and this PDU can be used to store the new request packet, and send it.
    // In general, you will use protocol's buffer instead another buffer.
    // It is useful in applications, that have no enough RAM space.
    // NOTE: In cae of ASCII protocols you should not use this PDU.
    struct modbus_pdu_t* tx_pdu;
/*
    NOTE:
        [high_level_context, recv_packet, error] variables
        are filled by high level.

        [low_level_context, send_packet, tx_pdu] variables
        are filled by low level.
*/
};

int modbus_proto_send_packet(struct modbus_protocol_t* _proto,
                             int _slave_addr,
                             const struct modbus_const_pdu_t* _pkt);

void modbus_proto_recv_packet(struct modbus_protocol_t* _proto,
                              int _slave_addr,
                              const struct modbus_const_pdu_t* _pkt);

void modbus_proto_error(struct modbus_protocol_t* _proto,
                        int _errno);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_MASTER_PROTOCOL_H
