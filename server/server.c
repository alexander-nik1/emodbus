
#include <emodbus/server/server.h>
#include <emodbus/base/modbus_errno.h>
#include <string.h>

#define build_exception_pdu(_ssrv_, _errno_)  \
    emb_build_exception_pdu((_ssrv_)->tx_pdu, (_ssrv_)->rx_pdu->function, _errno_)

#define DO_EVENT(_ssrv_, _event_, _data_) \
    if((_ssrv_)->on_event)          \
        (_ssrv_)->on_event((_ssrv_), (_event_), _data_)

static void emb_super_server_on_receive_req(void* _user_data,
                                            int _slave_addr,
                                            emb_const_pdu_t* _req) {

    struct emb_super_server_t* ssrv = (struct emb_super_server_t*)_user_data;
    struct emb_server_t* srv;
    emb_srv_function_t func;
    uint8_t res;

    DO_EVENT(ssrv, embsev_on_receive_pkt, 0);

    if(!ssrv->get_server) {
        DO_EVENT(ssrv, embsev_no_srv, 0);
        return;
    }

    if(!(srv = ssrv->get_server(ssrv, _slave_addr))) {
        DO_EVENT(ssrv, embsev_no_srv, 0);
        return;
    }

    // ok, here we are have the found server.
    // this means, that a response is should be sent.
    do {
        if(!srv->get_function) {
            build_exception_pdu(ssrv, MBE_ILLEGAL_FUNCTION);
            DO_EVENT(ssrv, embsev_mb_exception, MBE_ILLEGAL_FUNCTION);
            break;
        }

        func = srv->get_function(srv, _req->function);

        if(!func) {
            build_exception_pdu(ssrv, MBE_ILLEGAL_FUNCTION);
            DO_EVENT(ssrv, embsev_mb_exception, MBE_ILLEGAL_FUNCTION);
            break;
        }

        if((res = func(ssrv, srv))) {
            build_exception_pdu(ssrv, res);
            DO_EVENT(ssrv, embsev_mb_exception, res);
            break;
        }

    } while(0);

    emb_proto_send_packet(ssrv->proto, _slave_addr, MB_CONST_PDU(ssrv->tx_pdu));
    DO_EVENT(ssrv, embsev_resp_sent, 0);
}

static void emb_super_server_on_error(void* _user_data, int _errno) {
    struct emb_super_server_t* ssrv = (struct emb_super_server_t*)_user_data;
}

void emb_super_server_init(struct emb_super_server_t* _ssrv) { }

void emb_super_server_set_proto(struct emb_super_server_t* _ssrv,
                                struct emb_protocol_t* _proto) {
    if(_proto) {
        _ssrv->proto = _proto;
        _proto->high_level_context = _ssrv;
        _proto->recv_packet = emb_super_server_on_receive_req;
        _proto->error = emb_super_server_on_error;
        _proto->rx_pdu = _ssrv->rx_pdu;
    }
}
