
#include <emodbus/client/client.h>
#include <emodbus/base/modbus_errno.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

/*!
 * \file
 * \brief Realisation of modbus client (master) side.
 *
 */

#define CLIENT_REQ_ON_ERROR(_req_, _errno_)        \
    if((_req_->procs) && (_req_->procs->on_error)) \
        _req_->procs->on_error(_req_, _errno_)

static void emb_client_on_receive_pkt(void* _user_data,
                           int _slave_addr,
                           emb_const_pdu_t* _pkt) {

    struct emb_client_t* cli = (struct emb_client_t*)_user_data;
    struct emb_client_request_t* req = cli->curr_req;
    const struct emb_client_function_i* func;
    int res;

    cli->curr_req = (struct emb_client_request_t*)0;

    cli->state = mt_state_no_task;

    do {
        if(!req) {
            // something wrong ...
            break;
        }

        if(cli->curr_addr != _slave_addr) {
            CLIENT_REQ_ON_ERROR(req, -modbus_resp_wrong_address);
            break;
        }

        if(req->req_pdu->function != _pkt->function) {
            CLIENT_REQ_ON_ERROR(req, -modbus_resp_wrong_func);
            break;
        }

        if(req->procs && req->procs->on_response)
            req->procs->on_response(req, _slave_addr);
    }
    while(0);
}

static void emb_client_on_error(void* _user_data, int _errno) {
    struct emb_client_t* cli = (struct emb_client_t*)_user_data;
    struct emb_client_request_t* req = cli->curr_req;
    if(req) {
        cli->curr_req = (struct emb_client_request_t*)0;
        cli->state = mt_state_no_task;
        CLIENT_REQ_ON_ERROR(req, _errno);
    }
}

void emb_client_init(struct emb_client_t *_cli) {
    emb_client_set_proto(_cli, _cli->proto);
    _cli->curr_req = (struct emb_client_request_t*)0;
}

void emb_client_wait_timeout(struct emb_client_t* _cli) {
    struct emb_client_request_t* req = _cli->curr_req;
    if(req) {
        _cli->state = mt_state_no_task;
        _cli->curr_req = (struct emb_client_request_t*)0;
//        CLIENT_REQ_ON_ERROR(req, -modbus_resp_timeout);
    }
}

int emb_client_do_request(struct emb_client_t* _cli,
                          int _slave_addr,
                          struct emb_client_request_t* _req) {

    if(_cli->state != mt_state_no_task)
        return -EBUSY;
    _cli->curr_req = _req;
    _cli->state = mt_state_sending_req;
    _cli->curr_addr = _slave_addr;
    _cli->curr_req = _req;
    _cli->proto->rx_pdu = _req->resp_pdu;
    emb_proto_send_packet(_cli->proto, _slave_addr, _req->req_pdu);
    _cli->state = mt_state_wait_resp;
    return 0;
}

void emb_client_set_proto(struct emb_client_t* _cli,
                          struct emb_protocol_t* _proto) {
    if(_proto) {
        _cli->proto = _proto;
        _proto->high_level_context = _cli;
        _proto->recv_packet = emb_client_on_receive_pkt;
        _proto->error = emb_client_on_error;
    }
}
