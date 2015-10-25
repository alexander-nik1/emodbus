
#include "modbus_proto.h"

/*!
 * \file
 * \brief The realization of the protocol abstractions.
 *
 * This file contains an abstractions of modbus protocols
 * like: RTU, ASCII, TCP and so on. Description of this
 * functions see in modbus_proto.h
 */

int emb_proto_send_packet(struct emb_protocol_t* _proto,
                          int _slave_addr,
                          emb_const_pdu_t *_pkt) {
    return _proto->send_packet(_proto->low_level_context,
                                            _slave_addr,
                                            _pkt);
}

void emb_proto_recv_packet(struct emb_protocol_t* _proto,
                           int _slave_addr,
                           emb_const_pdu_t *_pkt) {
    _proto->recv_packet(_proto->high_level_context,
                                     _slave_addr,
                                     _pkt);
}

void emb_proto_error(struct emb_protocol_t* _proto,
                     int _errno) {
    /**
     * Because this function is not obligatory, this pointer can be zero.
     * And we should check it.
     */
    if(_proto->error)
        _proto->error(_proto->high_level_context, _errno);
}
