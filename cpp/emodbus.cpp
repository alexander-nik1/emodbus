
#include <emodbus/emodbus.hpp>
#include <emodbus/base/modbus_errno.h>
#include <emodbus/client/read_holding_regs.h>
#include <emodbus/client/write_multi_regs.h>

namespace emb {

// *******************************************************************************
// pdu_t

pdu_t::pdu_t(unsigned int _sz) {
    resize(_sz);
}

void pdu_t::resize(unsigned int _size) {
    buffer.resize(_size);
    emb_pdu_t::max_size = buffer.size();
    emb_pdu_t::data = &buffer[0];
}

pdu_t::operator emb_pdu_t* () {
    return (emb_pdu_t*) (this);
}

pdu_t::operator emb_const_pdu_t* () const {
    return (emb_const_pdu_t*) (this);
}

// *******************************************************************************
// sync_client_t

sync_client_t::sync_client_t() {

    client.user_data = this;

    emb_client_init(&client);
    emb_client_add_function(&client, &read_holding_regs_interface);
    emb_client_add_function(&client, &write_multi_regs_interface);
}

int sync_client_t::do_request(int _server_addr,
                    unsigned int _timeout,
                    emb_const_pdu_t* _request,
                    emb_pdu_t *_response) {
    int res;

    req.req_pdu = _request;
    req.resp_pdu = _response;
    req.procs = &procs;
    req.user_data = this;

    if((res = emb_client_do_request(&client, _server_addr, &req)) != 0) {
        return res;
    }

    res = emb_client_start_wait(_timeout);

    if(!res) {
        return result;
    }
    else if(res == 1) {
        emb_client_wait_timeout(&client);
        return -modbus_resp_timeout;
    }
    else {
        return res;
    }
}

void sync_client_t::set_proto(struct emb_protocol_t* _proto) {
    emb_client_set_proto(&client, _proto);
}

void sync_client_t::emb_on_response(struct emb_client_request_t* _req, int _slave_addr) {
    sync_client_t* _this = (sync_client_t*)_req->user_data;
    _this->result = 0;
    _this->emb_client_stop_wait();
}

void sync_client_t::emb_on_error(struct emb_client_request_t* _req, int _errno) {
    sync_client_t* _this = (sync_client_t*)_req->user_data;
    _this->result = _errno;
    _this->emb_client_stop_wait();
}


struct emb_client_req_procs_t sync_client_t::procs = {
    sync_client_t::emb_on_response,
    sync_client_t::emb_on_error
};


}; // namespace emb
