
#include "modbus_proto.h"

int modbus_proto_send_packet(struct modbus_protocol_t* _proto,
                             const void* _pkt,
                             unsigned int _pkt_size) {

    return _proto->modbus_proto_send_packet(_proto->send_user_data,
                                            _pkt, _pkt_size);
}

void modbus_proto_recv_packet(struct modbus_protocol_t* _proto,
                              const void* _pkt,
                              unsigned int _pkt_size) {

    _proto->modbus_proto_recv_packet(_proto->recv_user_data,
                                     _pkt, _pkt_size);
}
