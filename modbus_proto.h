
#ifndef MODBUS_MASTER_PROTOCOL_H
#define MODBUS_MASTER_PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*modbus_proto_send_packet_t)(void* _user_data,
                                          const void* _pkt,
                                          unsigned int _pkt_size);

typedef void (*modbus_proto_recv_packet_t)(void* _user_data,
                                           const void* _pkt,
                                           unsigned int _pkt_size);

struct modbus_protocol_t {

    void* send_user_data;
    void* recv_user_data;

    modbus_proto_send_packet_t modbus_proto_send_packet;
    modbus_proto_recv_packet_t modbus_proto_recv_packet;
};

int modbus_proto_send_packet(struct modbus_protocol_t* _proto,
                             const void* _pkt, unsigned int _pkt_size);

void modbus_proto_recv_packet(struct modbus_protocol_t* _proto,
                              const void* _pkt, unsigned int _pkt_size);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_MASTER_PROTOCOL_H
