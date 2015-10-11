 
#include "client_base.h"
#include <errno.h>
#include "../base/modbus_errno.h"
#include <string.h>

static void emb_cli_send_error(struct emb_client_t* _cli, int _errno) {
    if(_cli->on_error)
        _cli->on_error(_cli->user_data, _errno);
}

static void emb_client_recv_packet(void* _user_data,
                    int _slave_addr,
                    const struct modbus_const_pdu_t* _pkt) {

    struct emb_client_t* cli = (struct emb_client_t*)_user_data;

    const struct emb_client_function_i* func;
    int res;

    do {
        if(!cli->current_pdu) {
            emb_cli_send_error(cli, modbus_resp_without_req);
            break;
        }

        func = cli->functions[cli->current_pdu->function];

        if(!func) {
            emb_cli_send_error(cli, modbus_no_such_function);
            break;
        }

        if((res = func->check_answer(cli->current_pdu, _pkt))) {
            emb_cli_send_error(cli, res);
            break;
        }

        cli->on_response(cli->user_data, _slave_addr, _pkt);
    }
    while(0);

    cli->current_pdu = (struct modbus_const_pdu_t*)0;
}

static void emb_client_error(void* _user_data, int _errno) {

    struct emb_client_t* cli = (struct emb_client_t*)_user_data;
    emb_cli_send_error(cli, _errno);
    cli->current_pdu = (struct modbus_const_pdu_t*)0;
}

void emb_client_initialize(struct emb_client_t* _cli) {
    memset(_cli->functions, 0, sizeof(void*)*EMB_CLI_MAX_FUNCTIONS);
    _cli->current_pdu = (struct modbus_const_pdu_t*)0;
    _cli->protocol->high_level_context = _cli;
    _cli->protocol->recv_packet = emb_client_recv_packet;
    _cli->protocol->error = emb_client_error;
}

int emb_client_add_function(struct emb_client_t *_cli,
                            uint8_t _fucntion,
                            const struct emb_client_function_i* _func_i) {
    if(_fucntion < EMB_CLI_MAX_FUNCTIONS) {
        _cli->functions[_fucntion] = _func_i;
        return 0;
    }
    else
        return -EINVAL;
}

int emb_client_remove_function(struct emb_client_t* _cli, uint8_t _fucntion) {
    if(_fucntion < EMB_CLI_MAX_FUNCTIONS) {
        _cli->functions[_fucntion] = (struct emb_client_function_i*)0;
        return 0;
    }
    else
        return -EINVAL;
}

int emb_client_send_request(struct emb_client_t* _cli, int _server_addr, const struct modbus_const_pdu_t* _pdu) {
    _cli->current_pdu = _pdu;
    return modbus_proto_send_packet(_cli->protocol, _server_addr, _pdu);
}
