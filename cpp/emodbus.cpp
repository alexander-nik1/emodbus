
#include <emodbus/emodbus.hpp>
#include <emodbus/base/add/container_of.h>
#include <emodbus/base/modbus_errno.h>

#include <emodbus/client/read_holding_regs.h>
#include <emodbus/client/write_mask_reg.h>
#include <emodbus/client/write_single_reg.h>
#include <emodbus/client/write_multi_regs.h>
#include <emodbus/client/read_file_record.h>
#include <emodbus/client/write_file_record.h>
#include <emodbus/client/read_fifo.h>

#include <string.h>

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


read_regs_t::read_regs_t() { }

void read_regs_t::build_req(uint16_t _starting_address, uint16_t _quantity) {
    int res;

    req.resize(emb_read_hold_regs_calc_req_data_size());
    ans.resize(emb_read_hold_regs_calc_answer_data_size(_quantity));

    if((res = emb_read_hold_regs_make_req(req, _starting_address, _quantity)))
        throw res;
}

uint16_t read_regs_t::get_req_starting_addr() const {
    return emb_read_hold_regs_get_starting_addr(req);
}

uint16_t read_regs_t::get_req_quantity() const {
    return emb_read_hold_regs_get_quantity(req);
}

uint16_t read_regs_t::get_answer_reg(uint16_t _offset) const {
    return emb_read_hold_regs_get_reg(ans, _offset);
}

uint16_t read_regs_t::get_answer_quantity() const {
    return emb_read_hold_regs_get_regs_n(ans);
}

// *******************************************************************************
// write_reg_t

write_reg_t::write_reg_t() { }

void write_reg_t::build_req(uint16_t _address, uint16_t _value) {
    int res;
    req.resize(emb_write_reg_calc_req_data_size());
    ans.resize(emb_write_reg_calc_answer_data_size());

    if((res = emb_write_reg_make_req(req, _address, _value)))
        throw res;
}

uint16_t write_reg_t::get_req_address() const {
    return emb_write_reg_get_address(req);
}

uint16_t write_reg_t::get_req_value() const {
    return emb_write_reg_get_value(req);
}

uint16_t write_reg_t::get_answer_address() const {
    return emb_write_reg_get_address(ans);
}

uint16_t write_reg_t::get_answer_value() const {
    return emb_write_reg_get_value(ans);
}

// *******************************************************************************
// write_regs_t

write_regs_t::write_regs_t() { }

void write_regs_t::build_req(uint16_t _address, uint16_t _quantity,
                             const uint16_t* _data) {

    int res;
    req.resize(emb_write_regs_calc_req_data_size(_quantity));
    ans.resize(emb_write_regs_calc_answer_data_size());

    if((res = emb_write_regs_make_req(req, _address, _quantity, _data)))
        throw res;
}

uint16_t write_regs_t::get_req_address() const {
    return emb_write_regs_get_req_address(req);
}

uint16_t write_regs_t::get_req_quantity() const {
    return emb_write_regs_get_req_quantity(req);
}

uint16_t write_regs_t::get_req_data(uint16_t _offset) const {
    return emb_write_regs_get_req_data(req, _offset);
}

uint16_t write_regs_t::get_answer_address() const {
    return emb_write_regs_get_answer_address(ans);
}

uint16_t write_regs_t::get_answer_quantity() const {
    return emb_write_regs_get_answer_quantity(ans);
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
// read_fifo_t

read_fifo_t::read_fifo_t() { }

void read_fifo_t::build_req(uint16_t _starting_address) {
    int res;
    req.resize(emb_read_fifo_calc_req_data_size());
    ans.resize(emb_read_fifo_calc_answer_data_size());

    if((res = emb_read_fifo_make_req(req, _starting_address)))
        throw res;
}

uint16_t read_fifo_t::get_answer_regs_count() const {
    return emb_read_fifo_regs_count(ans);
}

uint16_t read_fifo_t::get_answer_data(uint16_t _offset) const {
    return emb_read_fifo_get_data(ans, _offset);
}

// *******************************************************************************
// sync_client_t

sync_client_t::sync_client_t() {

    memset(&client, 0, sizeof(struct emb_client_t));

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

// *******************************************************************************
// aync_client_t

aync_client_t::aync_client_t() {

    memset(&client, 0, sizeof(struct emb_client_t));

    client.user_data = this;

    emb_client_init(&client);
}

int aync_client_t::do_request(int _server_addr,
                              int _req_id,
                              emb_const_pdu_t* _request,
                              emb_pdu_t *_response,
                              claabacker_t* _callbacker) {

    req.req_pdu = _request;
    req.resp_pdu = _response;
    req.procs = &procs;
    req.user_data = this;
    curr_callbacker = _callbacker;
    curr_req_id = _req_id;

    return emb_client_do_request(&client, _server_addr, &req);
}

void aync_client_t::set_proto(struct emb_protocol_t* _proto) {
    emb_client_set_proto(&client, _proto);
}

void aync_client_t::answer_timeout() {
    emb_client_wait_timeout(&client);
}

void aync_client_t::emb_on_response(struct emb_client_request_t* _req, int _slave_addr) {
    aync_client_t* _this = (aync_client_t*)_req->user_data;
    if(_this->curr_callbacker)
        _this->curr_callbacker->emb_client_on_response(_req, _this->curr_req_id, _slave_addr);
}

void aync_client_t::emb_on_error(struct emb_client_request_t* _req, int _errno) {
    aync_client_t* _this = (aync_client_t*)_req->user_data;
    if(_this->curr_callbacker)
        _this->curr_callbacker->emb_client_on_error(_req, _this->curr_req_id, _errno);
}


struct emb_client_req_procs_t aync_client_t::procs = {
    aync_client_t::emb_on_response,
    aync_client_t::emb_on_error
};

// *******************************************************************************
// server_holdings_t

server_holdings_t::server_holdings_t() {
    set_funcs();
}

server_holdings_t::server_holdings_t(uint16_t _start, uint16_t _size) {
    h.start = _start;
    h.size = _size;
    set_funcs();
}

void server_holdings_t::set_start(uint16_t _start)
{ h.start = _start; }

void server_holdings_t::set_size(uint16_t _size)
{ h.size = _size; }

uint8_t server_holdings_t::on_read_regs(uint16_t _offset,
                             uint16_t _quantity,
                             uint16_t* _pvalues)
{ return 0; }

uint8_t server_holdings_t::on_write_regs(uint16_t _offset,
                              uint16_t _quantity,
                              const uint16_t* _pvalues)
{ return 0; }

void server_holdings_t::set_funcs() {
    h.read_regs = read_regs;
    h.write_regs = write_regs;
}

uint8_t server_holdings_t::read_regs(struct emb_srv_holdings_t* _rr,
                                     uint16_t _offset,
                                     uint16_t _quantity,
                                     uint16_t* _pvalues) {
    server_holdings_t* _this = container_of(_rr, server_holdings_t, h);
    return _this->on_read_regs(_offset, _quantity, _pvalues);
}

uint8_t server_holdings_t::write_regs(struct emb_srv_holdings_t* _rr,
                                      uint16_t _offset,
                                      uint16_t _quantity,
                                      const uint16_t* _pvalues) {
    server_holdings_t* _this = container_of(_rr, server_holdings_t, h);
    return _this->on_write_regs(_offset, _quantity, _pvalues);
}

// *******************************************************************************
// server_file_t

server_file_t::server_file_t() {
    set_funcs();
}

server_file_t::server_file_t(uint16_t _fileno/*, uint16_t _start, uint16_t _size*/) {
    f.fileno = _fileno;
//    f.start = _start;
//    f.size = _size;
    set_funcs();
}

void server_file_t::set_fileno(uint16_t _fileno)
{ f.fileno = _fileno; }

//void server_file_t::set_start(uint16_t _start)
//{ f.start = _start; }

//void server_file_t::set_size(uint16_t _size)
//{ f.size = _size; }

uint8_t server_file_t::on_read_file(uint16_t _offset,
                             uint16_t _quantity,
                             uint16_t* _pvalues)
{ return 0; }

uint8_t server_file_t::on_write_file(uint16_t _offset,
                              uint16_t _quantity,
                              const uint16_t* _pvalues)
{ return 0; }

void server_file_t::set_funcs() {
    f.read_file = read_file;
    f.write_file = write_file;
}

uint8_t server_file_t::read_file(emb_srv_file_t *_f,
                                 uint16_t _offset,
                                 uint16_t _quantity,
                                 uint16_t* _pvalues) {
    server_file_t* _this = container_of(_f, server_file_t, f);
    return _this->on_read_file(_offset, _quantity, _pvalues);
}

uint8_t server_file_t::write_file(emb_srv_file_t *_f,
                                  uint16_t _offset,
                                  uint16_t _quantity,
                                  const uint16_t* _pvalues) {
    server_file_t* _this = container_of(_f, server_file_t, f);
    return _this->on_write_file(_offset, _quantity, _pvalues);
}

// *******************************************************************************
// server_t

server_t::server_t(int _address) : address(_address) {
    memset(&srv, 0, sizeof(struct emb_server_t));
    srv.get_function = get_function;
    srv.get_holdings = get_holdings;
    srv.get_file = get_file;
}

int server_t::addr() const { return address; }

bool server_t::add_function(uint8_t _func_no, emb_srv_function_t _func) {
    for(func_iter i=functions.begin(); i != functions.end(); ++i) {
        if(i->func_no == _func_no)
            return false;
    }
    function_t f = { _func_no, _func };
    functions.push_back(f);
    return true;
}

#define IS_POINT_IN_RANGE(_range_start_, _range_end_, _point_) \
    (((_range_start_) <= (_point_)) && ((_point_) < (_range_end_)))

static bool emb_is_ranges_cross(uint16_t _start1,
                         uint16_t _start2,
                         uint16_t _size1,
                         uint16_t _size2) {

    const uint16_t end1 = _start1 + _size1;
    const uint16_t end2 = _start2 + _size2;

    if( IS_POINT_IN_RANGE(_start1, end1, _start2) ||
        IS_POINT_IN_RANGE(_start1, end1, end2) ||
        IS_POINT_IN_RANGE(_start2, end2, _start1) ||
        IS_POINT_IN_RANGE(_start2, end2, end1))
        return true;

    return false;
}

bool server_t::add_holdings(server_holdings_t& _holdings) {
    for(holdnigs_iter i=holdings.begin(); i != holdings.end(); ++i) {
        if( emb_is_ranges_cross((*i)->h.start, (*i)->h.size, _holdings.h.start, _holdings.h.size) )
            return false;
    }
    holdings.push_back(&_holdings);
    return true;
}

bool server_t::add_file(server_file_t& _file) {
    for(files_iter i=files.begin(); i != files.end(); ++i) {
        if((*i)->f.fileno == _file.f.fileno)
            //if(emb_is_ranges_cross((*i)->f.start, (*i)->f.size, _file.f.start, _file.f.size) )
                return false;
    }
    files.push_back(&_file);
    return true;
}

emb_srv_function_t server_t::get_function(struct emb_server_t* _srv, uint8_t _func) {
    server_t* _this = container_of(_srv, server_t, srv);
    for(func_iter i=_this->functions.begin(); i != _this->functions.end(); ++i) {
        if(i->func_no == _func)
            return i->func;
    }
    return NULL;
}

struct emb_srv_holdings_t* server_t::get_holdings(struct emb_server_t* _srv, uint16_t _begin) {
    server_t* _this = container_of(_srv, server_t, srv);
    for(holdnigs_iter i=_this->holdings.begin(); i != _this->holdings.end(); ++i) {
        const uint16_t start = (*i)->h.start;
        const uint16_t end = start + (*i)->h.size - 1;
        if((start <= _begin) && (_begin <= end))
            return &((*i)->h);
    }
    return NULL;
}

struct emb_srv_file_t* server_t::get_file(struct emb_server_t* _srv, uint16_t _fileno/*, uint16_t _begin*/) {
    server_t* _this = container_of(_srv, server_t, srv);
    for(files_iter i=_this->files.begin(); i != _this->files.end(); ++i) {
        if((*i)->f.fileno == _fileno)
            return &((*i)->f);
    }
    return NULL;
}

// *******************************************************************************
// super_server_t

super_server_t::super_server_t() {

    memset(&ssrv, 0, sizeof(struct emb_super_server_t));

    ssrv.get_server = _get_server;

    rx_pdu.resize(MAX_PDU_DATA_SIZE);
    tx_pdu.resize(MAX_PDU_DATA_SIZE);

    ssrv.rx_pdu = &rx_pdu;
    ssrv.tx_pdu = &tx_pdu;
}

void super_server_t::set_proto(struct emb_protocol_t* _proto) {
    emb_super_server_set_proto(&ssrv, _proto);
}

bool super_server_t::add_server(server_t& _srv) {
    for(srv_iter i=servers.begin(); i != servers.end(); ++i) {
        if((*i)->addr() == _srv.addr())
            return false;
    }
    servers.push_back(&_srv);
    return true;
}

struct emb_server_t* super_server_t::_get_server(struct emb_super_server_t* _ssrv,
                                        uint8_t _address) {
    super_server_t* _this = container_of(_ssrv, super_server_t, ssrv);
    for(srv_iter i=_this->servers.begin(); i != _this->servers.end(); ++i) {
        if((*i)->addr() == _address)
            return &(*i)->srv;
    }
    return NULL;
}

} // namespace emb
