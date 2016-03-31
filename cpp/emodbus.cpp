
#include <emodbus/emodbus.hpp>
#include <emodbus/base/modbus_errno.h>
#include <emodbus/client/read_holding_regs.h>
#include <emodbus/client/write_mask_reg.h>
#include <emodbus/client/write_multi_regs.h>

namespace emb {

// *******************************************************************************
// pdu_t

pdu_t::pdu_t() {
    emb_pdu_t::max_size = 0;
    emb_pdu_t::data_size = 0;
}

pdu_t::pdu_t(unsigned int _sz) {
    resize(_sz);
}

void pdu_t::resize(unsigned int _size) {
    if(buffer.size() < _size) {
        buffer.resize(_size);
        emb_pdu_t::max_size = buffer.size();
        emb_pdu_t::data = &buffer[0];
    }
}

pdu_t::operator emb_pdu_t* () {
    return (emb_pdu_t*) (this);
}

pdu_t::operator emb_const_pdu_t* () const {
    return (emb_const_pdu_t*) (this);
}

// *******************************************************************************
// read_hold_regs_t


read_hold_regs_t::read_hold_regs_t() { }

void read_hold_regs_t::build_req(uint16_t _starting_address, uint16_t _quantity) {
    int res;

    req.resize(emb_read_hold_regs_calc_req_data_size());
    ans.resize(emb_read_hold_regs_calc_answer_data_size(_quantity));

    if((res = emb_read_hold_regs_make_req(req, _starting_address, _quantity)))
        throw res;
}

uint16_t read_hold_regs_t::get_req_starting_addr() const {
    return emb_read_hold_regs_get_starting_addr(req);
}

uint16_t read_hold_regs_t::get_req_quantity() const {
    return emb_read_hold_regs_get_quantity(req);
}

uint16_t read_hold_regs_t::get_answer_reg(uint16_t _offset) const {
    return emb_read_hold_regs_get_reg(ans, _offset);
}

uint16_t read_hold_regs_t::get_answer_quantity() const {
    return emb_read_hold_regs_get_regs_n(ans);
}

// *******************************************************************************
// write_mask_reg_t

write_mask_reg_t::write_mask_reg_t() { }

void write_mask_reg_t::build_req(uint16_t _address,
                                 uint16_t _and_mask,
                                 uint16_t _or_mask) {
    int res;
    req.resize(emb_write_mask_reg_calc_req_data_size());
    ans.resize(emb_write_mask_reg_calc_answer_data_size());

    if((res = emb_write_mask_reg_make_req(req, _address, _and_mask, _or_mask)))
        throw res;
}

uint16_t write_mask_reg_t::write_mask_reg_t::get_req_address() const {
    return emb_write_mask_reg_get_address(req);
}

uint16_t write_mask_reg_t::write_mask_reg_t::get_req_and_mask() const {
    return emb_write_mask_reg_get_and_mask(req);
}

uint16_t write_mask_reg_t::write_mask_reg_t::get_req_or_mask() const {
    return emb_write_mask_reg_get_or_mask(req);
}

uint16_t write_mask_reg_t::write_mask_reg_t::get_answer_address() const {
    return emb_write_mask_reg_get_address(ans);
}

uint16_t write_mask_reg_t::write_mask_reg_t::get_answer_and_mask() const {
    return emb_write_mask_reg_get_and_mask(ans);
}

uint16_t write_mask_reg_t::write_mask_reg_t::get_answer_or_mask() const {
    return emb_write_mask_reg_get_or_mask(ans);
}


// *******************************************************************************
// sync_client_t

sync_client_t::sync_client_t() {

    client.user_data = this;

    emb_client_init(&client);
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


} // namespace emb
