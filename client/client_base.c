 
#include "client_base.h"
#include <errno.h>
#include "../base/modbus_errno.h"
#include <string.h>

enum {
    emb_cli_state_default,
    emb_cli_state_req_sending,
    emb_cli_state_wait_resp,
    emb_cli_state_resp_ok,
    emb_cli_state_resp_fail
};

static void emb_client_recv_packet(void* _user_data,
                    int _slave_addr,
                    const struct modbus_const_pdu_t* _pkt) {

    struct emb_client_t* cli = (struct emb_client_t*)_user_data;

    const struct emb_client_function_i* func;
    int res;

    do {
        if(!cli->current_request) {
            cli->error_code = -modbus_resp_without_req;
            cli->state = emb_cli_state_resp_fail;
            break;
        }

        if(cli->curr_req_server_addr != _slave_addr) {
            cli->error_code = -modbus_resp_wrong_address;
            cli->state = emb_cli_state_resp_fail;
            break;
        }

        func = cli->functions[cli->current_request->function];

        if(!func) {
            cli->error_code = -modbus_no_such_function;
            cli->state = emb_cli_state_resp_fail;
            break;
        }

        if((res = func->check_answer(cli->current_request, _pkt))) {
            cli->error_code = res;
            cli->state = emb_cli_state_resp_fail;
            break;
        }

        cli->error_code = 0;
        cli->current_response = _pkt;
        cli->state = emb_cli_state_resp_ok;
    }
    while(0);

    cli->resp_timeout_mutex.unlock(cli->resp_timeout_mutex.user_data);
}

static void emb_client_error(void* _user_data, int _errno) {

    struct emb_client_t* cli = (struct emb_client_t*)_user_data;
    cli->error_code = _errno;
    cli->current_request = NULL;
    cli->current_response = NULL;
    if(cli->state == emb_cli_state_wait_resp)
        cli->state = emb_cli_state_resp_fail;
    cli->resp_timeout_mutex.unlock(cli->resp_timeout_mutex.user_data);
}

void emb_client_initialize(struct emb_client_t* _cli) {
    memset(_cli->functions, 0, sizeof(void*)*EMB_CLI_MAX_FUNCTIONS);
    _cli->current_request = NULL;
    _cli->current_response = NULL;
    _cli->state = emb_cli_state_default;
}

void emb_client_set_proto(struct emb_client_t* _cli,
                          struct modbus_protocol_t* _proto) {
    _cli->protocol = _proto;
    _proto->high_level_context = _cli;
    _proto->recv_packet = emb_client_recv_packet;
    _proto->error = emb_client_error;
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
        _cli->functions[_fucntion] = NULL;
        return 0;
    }
    else
        return -EINVAL;
}

int emb_client_do_request(struct emb_client_t* _cli,
                            int _server_addr,
                            unsigned int _timeout,
                            const struct modbus_const_pdu_t* _request,
                            const struct modbus_const_pdu_t**_response) {
    int res = 0;

    *_response = NULL;

    if(_cli->state != emb_cli_state_default)
        return -EBUSY;

    _cli->current_request = _request;
    _cli->curr_req_server_addr = _server_addr;

    _cli->state = emb_cli_state_req_sending;

    if((res = modbus_proto_send_packet(_cli->protocol, _server_addr, _request)))
        return res;

    _cli->state = emb_cli_state_wait_resp;

    _cli->resp_timeout_mutex.lock_timeout(_cli->resp_timeout_mutex.user_data,
                                          _timeout);

    switch(_cli->state) {

        case emb_cli_state_resp_ok:
            *_response = _cli->current_response;
            res = 0;
            break;

        case emb_cli_state_resp_fail:
            res = _cli->error_code;
            break;

        case emb_cli_state_wait_resp:
            res = -modbus_resp_timeout;
            break;

        default:
            res = -1;
    }

    _cli->state = emb_cli_state_default;

    return res;
}

