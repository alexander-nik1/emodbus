
#include "modbus_proto.h"

int modbus_proto_send_packet(struct modbus_protocol_t* _proto,
                             int _slave_addr,
                             const struct modbus_const_pdu_t *_pkt) {
    return _proto->send_packet(_proto->low_level_context,
                                            _slave_addr,
                                            _pkt);
}

void modbus_proto_recv_packet(struct modbus_protocol_t* _proto,
                              int _slave_addr,
                              const struct modbus_const_pdu_t *_pkt) {
    _proto->recv_packet(_proto->high_level_context,
                                     _slave_addr,
                                     _pkt);
}

void modbus_proto_error(struct modbus_protocol_t* _proto,
                        int _errno) {
    _proto->error(_proto->high_level_context, _errno);
}
