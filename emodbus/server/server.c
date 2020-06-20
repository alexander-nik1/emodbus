
#include <emodbus/server/server.h>
#include <emodbus/base/modbus_errno.h>
#include <string.h>

#define build_exception_pdu(_ssrv_, _errno_)  \
    emb_build_exception_pdu((_ssrv_)->tx_pdu, (_ssrv_)->rx_pdu->function, _errno_)

#define DO_EVENT(_ssrv_, _event_, _data_) \
    if((_ssrv_)->on_event)          \
        (_ssrv_)->on_event((_ssrv_), (_event_), _data_)

int emb_super_server_process_req(struct emb_super_server_t* _ssrv,
                                 const emb_adu_t* _rx_adu,
                                 emb_adu_t* _tx_adu)
{
    struct emb_server_t* srv;
    emb_srv_function_t func;
    uint8_t res;

    if(!(_ssrv && _rx_adu && _tx_adu))
        return -EINVAL;

    DO_EVENT(_ssrv, embsev_on_receive_pkt, 0);

    if(!_ssrv->get_server) {
        DO_EVENT(_ssrv, embsev_no_srv, 0);
        return 0;
    }

    if(!(srv = _ssrv->get_server(_ssrv, _rx_adu->server_id))) {
        DO_EVENT(_ssrv, embsev_no_srv, 0);
        return 0;
    }

    _ssrv->rx_pdu = &_rx_adu->pdu;
    _ssrv->tx_pdu = &_tx_adu->pdu;

    // ok, here we are have the found server.
    // this means, that a response is should be sent.
    do {
        if(!srv->get_function) {
            build_exception_pdu(_ssrv, MBE_ILLEGAL_FUNCTION);
            DO_EVENT(_ssrv, embsev_mb_exception, MBE_ILLEGAL_FUNCTION);
            break;
        }

        func = srv->get_function(srv, _rx_adu->pdu.function);

        if(!func) {
            build_exception_pdu(_ssrv, MBE_ILLEGAL_FUNCTION);
            DO_EVENT(_ssrv, embsev_mb_exception, MBE_ILLEGAL_FUNCTION);
            break;
        }

        if((res = func(_ssrv, srv))) {
            build_exception_pdu(_ssrv, res);
            DO_EVENT(_ssrv, embsev_mb_exception, res);
            break;
        }

    } while(0);

    if(!(srv->flags & EMB_SRV_BROADCAST_FLAG)) {

        _tx_adu->flags = _rx_adu->flags;
        _tx_adu->server_id = _rx_adu->server_id;
        _tx_adu->transaction_id = _rx_adu->transaction_id;

        DO_EVENT(_ssrv, embsev_resp_sent, 0);
        return 1;
    }

    return 0;
}

void emb_super_server_init(struct emb_super_server_t* _ssrv)
{
    if(_ssrv) {
        _ssrv->rx_pdu = NULL;
        _ssrv->tx_pdu = NULL;
    }
}
