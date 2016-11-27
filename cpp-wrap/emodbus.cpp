
#include <emodbus/emodbus.hpp>
#include <emodbus/base/add/container_of.h>
#include <emodbus/base/modbus_errno.h>

#include <emodbus/client/read_bits.h>
#include <emodbus/client/write_coil.h>
#include <emodbus/client/write_coils.h>
#include <emodbus/client/read_regs.h>
#include <emodbus/client/write_mask_reg.h>
#include <emodbus/client/write_single_reg.h>
#include <emodbus/client/write_multi_regs.h>
#include <emodbus/client/read_write_regs.h>
#include <emodbus/client/read_file_record.h>
#include <emodbus/client/write_file_record.h>
#include <emodbus/client/read_fifo.h>

#include <string.h>

namespace emb {

// *******************************************************************************
// regs_t

regs_t& regs_t::operator << (const int& _v) {
    push_back(_v);
    return *this;
}

regs_t& regs_t::operator << (const float& _v) {
    const uint16_t* p = (uint16_t*)&_v;
    push_back(p[0]);
    push_back(p[1]);
    return *this;
}

regs_t& regs_t::operator >> (int& _v) {
    _v = front();
    erase(begin());
    return *this;
}

regs_t& regs_t::operator >> (float& _v) {
    uint16_t* p = (uint16_t*)&_v;
    iterator i = begin();
    p[0] = *i++;
    p[1] = *i++;
    erase(begin(), i);
    return *this;
}

// *******************************************************************************
// pdu_t

pdu_t::pdu_t() {
    emb_pdu_t::max_size = 0;
    emb_pdu_t::data_size = 0;
    emb_pdu_t::data = NULL;
}

pdu_t::pdu_t(unsigned int _sz) {
    resize(_sz);
}

pdu_t::~pdu_t() {
    emb_pdu_t::max_size = 0;
    emb_pdu_t::data_size = 0;
    emb_pdu_t::data = NULL;
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

namespace client {

// *******************************************************************************
//  #####  #       ###   #####  #   #  #####
//  #      #        #    #      ##  #    #
//  #      #        #    #####  # # #    #
//  #      #        #    #      #  ##    #
//  #####  #####   ###   #####  #   #    #

// *******************************************************************************
// transaction_t

transaction_t::transaction_t() {
    tr.procs = &procs;
    tr.req_pdu = req;
    tr.resp_pdu = ans;
    tr.user_data = NULL;
}

void transaction_t::emb_transaction_on_response(int _slave_addr) { }
void transaction_t::emb_transaction_on_error(int _slave_addr, int _errno) { }

transaction_t::operator struct emb_client_transaction_t* () { return &tr; }

void transaction_t::emb_trans_on_response_(struct emb_client_transaction_t* _tr,
                                           int _slave_addr) {
    transaction_t* _this = container_of(_tr, transaction_t, tr);
    _this->emb_transaction_on_response(_slave_addr);
}

void transaction_t::emb_trans_on_error_(struct emb_client_transaction_t* _tr,
                                        int _slave_addr, int _errno) {
    transaction_t* _this = container_of(_tr, transaction_t, tr);
    _this->emb_transaction_on_error(_slave_addr, _errno);
}

struct emb_client_req_procs_t transaction_t::procs = {
    transaction_t::emb_trans_on_response_,
    transaction_t::emb_trans_on_error_
};

transact_base_t::transact_base_t()
    : self_tr(new transaction_t())
    , tr(self_tr) { }

transact_base_t::transact_base_t(transaction_t& _tr)
    : self_tr(NULL)
    , tr(&_tr) { }

transact_base_t::~transact_base_t() {
    if(self_tr)
        delete self_tr;
}

transact_base_t::operator transaction_t& ()
{ return *tr; }

transact_base_t::operator const transaction_t& () const
{ return *tr; }

// *******************************************************************************
// read_bits_t

read_bits_t::read_bits_t() { }
read_bits_t::read_bits_t(transaction_t& _tr) : transact_base_t(_tr) { }

void read_bits_t::build_req(enum EMB_RB_TYPE _type,
                            uint16_t _starting_address,
                            uint16_t _quantity) {
    int res;

    tr->req.resize(emb_read_bits_calc_req_data_size());
    tr->ans.resize(emb_read_bits_calc_answer_data_size(_quantity));

    if((res = emb_read_bits_make_req(tr->req, _type, _starting_address, _quantity)))
        throw res;
}

uint16_t read_bits_t::get_req_starting_addr() const {
    return emb_read_bits_get_starting_addr(tr->req);
}

uint16_t read_bits_t::get_req_quantity() const {
    return emb_read_bits_get_quantity(tr->req);
}

char read_bits_t::get_answer_bit(uint16_t _offset) const {
    return emb_read_bits_get_bit(tr->ans, _offset);
}

uint8_t read_bits_t::get_answer_byte(uint8_t _offset) const {
    return emb_read_bits_get_byte(tr->ans, _offset);
}

void read_bits_t::response_data(bool *_coils, unsigned int _size) const {
    uint16_t sz = emb_read_bits_get_quantity(tr->req);
    if(sz > _size)
        sz = _size;
    for(uint16_t i=0; i<sz; ++i) {
        _coils[i] = emb_read_bits_get_bit(tr->ans, i);
    }
}

// *******************************************************************************
// read_coils_t

read_coils_t::read_coils_t() { }
read_coils_t::read_coils_t(transaction_t& _tr) : read_bits_t(_tr) { }

void read_coils_t::build_req(uint16_t _starting_address, uint16_t _quantity)
{ read_bits_t::build_req(EMB_RB_COILS, _starting_address, _quantity); }

// *******************************************************************************
// read_discrete_inputs_t

read_discrete_inputs_t::read_discrete_inputs_t() { }
read_discrete_inputs_t::read_discrete_inputs_t(transaction_t& _tr) : read_bits_t(_tr) { }

void read_discrete_inputs_t::build_req(uint16_t _starting_address, uint16_t _quantity)
{ read_bits_t::build_req(EMB_RB_DISCRETE_INPUTS, _starting_address, _quantity); }

// *******************************************************************************
// write_coil_t

write_coil_t::write_coil_t() { }
write_coil_t::write_coil_t(transaction_t& _tr) : transact_base_t(_tr) { }

void write_coil_t::build_req(uint16_t _address, bool _value) {
    int res;

    tr->req.resize(emb_write_coil_calc_req_data_size());
    tr->ans.resize(emb_write_coil_calc_answer_data_size());

    if((res = emb_write_coil_make_req(tr->req, _address, _value)))
        throw res;
}

uint16_t write_coil_t::get_req_addr() const {
    return emb_write_coil_get_addr(tr->req);
}

// *******************************************************************************
// write_coils_t

write_coils_t::write_coils_t() { }
write_coils_t::write_coils_t(transaction_t& _tr) : transact_base_t(_tr) { }

void write_coils_t::build_req(uint16_t _starting_address,
                              uint16_t _quantity,
                              const bool *_pcoils) {
    int res;

    std::vector<uint8_t> coils;

    coils.resize((_quantity / 8) + ((_quantity & 7) ? 1 : 0));

    uint16_t quantity_counter = _quantity;

    for(int byte=0; quantity_counter != 0; ++byte) {
        coils[byte] = 0;
        const int n_bits = quantity_counter > 8 ? 8 : quantity_counter;
        for(int bit=0; bit<n_bits; ++bit) {
            if(_pcoils[byte*8 + bit]) {
                coils[byte] |= (1 << bit);
            }
        }
        quantity_counter -= n_bits;
    }

    tr->req.resize(emb_write_coils_calc_req_data_size(_quantity));
    tr->ans.resize(emb_write_coils_calc_answer_data_size());

    if((res = emb_write_coils_make_req(tr->req, _starting_address, _quantity, &coils[0])))
        throw res;
}

uint16_t write_coils_t::get_req_starting_addr() const {
    return emb_write_coils_get_starting_addr(tr->req);
}

uint16_t write_coils_t::get_req_quantity() const {
    return emb_write_coils_get_quantity(tr->req);
}

// *******************************************************************************
// read_hold_regs_t

read_regs_t::read_regs_t() : transact_base_t() { }
read_regs_t::read_regs_t(transaction_t &_tr) : transact_base_t(_tr) { }

void read_regs_t::build_req(enum EMB_RR_TYPE _subtype,
                            uint16_t _starting_address,
                            uint16_t _quantity) {
    int res;

    tr->req.resize(emb_read_regs_calc_req_data_size());
    tr->ans.resize(emb_read_regs_calc_answer_data_size(_quantity));

    if((res = emb_read_regs_make_req(tr->req,
                                     _subtype,
                                     _starting_address,
                                     _quantity)))
        throw res;
}

uint16_t read_regs_t::get_req_starting_addr() const {
    return emb_read_regs_get_starting_addr(tr->req);
}

uint16_t read_regs_t::get_req_quantity() const {
    return emb_read_regs_get_quantity(tr->req);
}

uint16_t read_regs_t::get_answer_reg(uint16_t _offset) const {
    return emb_read_regs_get_reg(tr->ans, _offset);
}

uint16_t read_regs_t::get_answer_quantity() const {
    return emb_read_regs_get_regs_n(tr->ans);
}

void read_regs_t::get_answer_regs(uint16_t* _p_data, uint16_t _offset, uint16_t _n_regs) {
    emb_read_regs_get_regs(tr->ans, _offset, _n_regs, _p_data);
}

void read_regs_t::get_answer_regs(regs_t& _res, uint16_t _offset, uint16_t _n_regs) {
    _res.resize(_n_regs);
    emb_read_regs_get_regs(tr->ans, _offset, _n_regs, &_res[0]);
}

// *******************************************************************************
// read_holding_regs_t

read_holding_regs_t::read_holding_regs_t() { }
read_holding_regs_t::read_holding_regs_t(transaction_t &_tr) : read_regs_t(_tr) { }

void read_holding_regs_t::build_req(uint16_t _starting_address, uint16_t _quantity)
{ read_regs_t::build_req(EMB_RR_HOLDINGS, _starting_address, _quantity); }

// *******************************************************************************
// read_input_regs_t

read_input_regs_t::read_input_regs_t() { }
read_input_regs_t::read_input_regs_t(transaction_t &_tr) : read_regs_t(_tr) { }

void read_input_regs_t::build_req(uint16_t _starting_address, uint16_t _quantity)
{ read_regs_t::build_req(EMB_RR_INPUTS, _starting_address, _quantity); }

// *******************************************************************************
// write_reg_t

write_reg_t::write_reg_t() { }
write_reg_t::write_reg_t(transaction_t& _tr) : transact_base_t(_tr) { }

void write_reg_t::build_req(uint16_t _address, uint16_t _value) {
    int res;
    tr->req.resize(emb_write_reg_calc_req_data_size());
    tr->ans.resize(emb_write_reg_calc_answer_data_size());

    if((res = emb_write_reg_make_req(tr->req, _address, _value)))
        throw res;
}

uint16_t write_reg_t::get_req_address() const {
    return emb_write_reg_get_address(tr->req);
}

uint16_t write_reg_t::get_req_value() const {
    return emb_write_reg_get_value(tr->req);
}

uint16_t write_reg_t::get_answer_address() const {
    return emb_write_reg_get_address(tr->ans);
}

uint16_t write_reg_t::get_answer_value() const {
    return emb_write_reg_get_value(tr->ans);
}

// *******************************************************************************
// write_regs_t

write_regs_t::write_regs_t() { }
write_regs_t::write_regs_t(transaction_t& _tr) : transact_base_t(_tr) { }

void write_regs_t::build_req(uint16_t _address, uint16_t _quantity,
                             const uint16_t* _data) {

    int res;
    tr->req.resize(emb_write_regs_calc_req_data_size(_quantity));
    tr->ans.resize(emb_write_regs_calc_answer_data_size());

    if((res = emb_write_regs_make_req(tr->req, _address, _quantity, _data)))
        throw res;
}

uint16_t write_regs_t::get_req_address() const {
    return emb_write_regs_get_req_address(tr->req);
}

uint16_t write_regs_t::get_req_quantity() const {
    return emb_write_regs_get_req_quantity(tr->req);
}

uint16_t write_regs_t::get_req_data(uint16_t _offset) const {
    return emb_write_regs_get_req_data(tr->req, _offset);
}

uint16_t write_regs_t::get_answer_address() const {
    return emb_write_regs_get_answer_address(tr->ans);
}

uint16_t write_regs_t::get_answer_quantity() const {
    return emb_write_regs_get_answer_quantity(tr->ans);
}

// *******************************************************************************
// read_write_regs_t

read_write_regs_t::read_write_regs_t() { }
read_write_regs_t::read_write_regs_t(transaction_t& _tr) : transact_base_t(_tr) { }

void read_write_regs_t::build_req(uint16_t _rd_address,
                                  uint16_t _rd_quantity,
                                  uint16_t _wr_address,
                                  uint16_t _wr_quantity,
                                  const uint16_t* _wr_data)
{
    int res;
    tr->req.resize(emb_rdwr_regs_calc_req_data_size(_wr_quantity));
    tr->ans.resize(emb_rdwr_regs_calc_answer_data_size(_rd_quantity));

    if((res = emb_rdwr_regs_make_req(tr->req, _rd_address, _rd_quantity,
                                     _wr_address, _wr_quantity, _wr_data)))
        throw res;
}

uint16_t read_write_regs_t::get_req_rd_address() const
{ return emb_rdwr_regs_get_req_rd_address(tr->req); }

uint16_t read_write_regs_t::get_req_rd_quantity() const
{ return emb_rdwr_regs_get_req_rd_quantity(tr->req); }

uint16_t read_write_regs_t::get_req_wr_address() const
{ return emb_rdwr_regs_get_req_wr_address(tr->req); }

uint16_t read_write_regs_t::get_req_wr_quantity() const
{ return emb_rdwr_regs_get_req_wr_quantity(tr->req); }

uint16_t read_write_regs_t::get_answer_rd_reg(uint16_t _offset) const
{ return emb_rdwr_regs_get_answ_reg(tr->ans, _offset); }

uint16_t read_write_regs_t::get_answer_rd_quantity() const
{ return emb_rdwr_regs_get_answ_regs_n(tr->ans); }

void read_write_regs_t::get_answer_rd_regs(uint16_t* _p_data, uint16_t _offset, uint16_t _n_regs)
{ emb_rdwr_regs_get_answ_regs(tr->ans, _offset, _n_regs, _p_data); }

void read_write_regs_t::get_answer_rd_regs(regs_t& _res, uint16_t _offset, uint16_t _n_regs)
{
    _res.resize(_n_regs);
    emb_rdwr_regs_get_answ_regs(tr->ans, _offset, _n_regs, &_res[0]);
}

// *******************************************************************************
// write_mask_reg_t

write_mask_reg_t::write_mask_reg_t() { }
write_mask_reg_t::write_mask_reg_t(transaction_t& _tr) : transact_base_t(_tr) { }

void write_mask_reg_t::build_req(uint16_t _address,
                                 uint16_t _and_mask,
                                 uint16_t _or_mask) {
    int res;
    tr->req.resize(emb_write_mask_reg_calc_req_data_size());
    tr->ans.resize(emb_write_mask_reg_calc_answer_data_size());

    if((res = emb_write_mask_reg_make_req(tr->req, _address, _and_mask, _or_mask)))
        throw res;
}

uint16_t write_mask_reg_t::write_mask_reg_t::get_req_address() const {
    return emb_write_mask_reg_get_address(tr->req);
}

uint16_t write_mask_reg_t::write_mask_reg_t::get_req_and_mask() const {
    return emb_write_mask_reg_get_and_mask(tr->req);
}

uint16_t write_mask_reg_t::write_mask_reg_t::get_req_or_mask() const {
    return emb_write_mask_reg_get_or_mask(tr->req);
}

uint16_t write_mask_reg_t::write_mask_reg_t::get_answer_address() const {
    return emb_write_mask_reg_get_address(tr->ans);
}

uint16_t write_mask_reg_t::write_mask_reg_t::get_answer_and_mask() const {
    return emb_write_mask_reg_get_and_mask(tr->ans);
}

uint16_t write_mask_reg_t::write_mask_reg_t::get_answer_or_mask() const {
    return emb_write_mask_reg_get_or_mask(tr->ans);
}

// *******************************************************************************
// read_file_t

read_file_t::subreq_t::subreq_t(uint16_t _fileno, uint16_t _record_no, uint16_t _length) {
    emb_read_file_req_t::file_number = _fileno;
    emb_read_file_req_t::record_number = _record_no;
    emb_read_file_req_t::record_length = _length;
}

read_file_t::answer_iterator_t::answer_iterator_t(emb_const_pdu_t *_anw)
    : ans(_anw)
    , iterator_(NULL) {
    reset_to_begin();
}

uint16_t read_file_t::answer_iterator_t::subanswer_quantity() const {
    return emb_read_file_subanswer_quantity(iterator_);
}

uint16_t read_file_t::answer_iterator_t::subanswer_data(int _i) const {
    if(_i < 0)
        _i = subanswer_quantity() + _i;
    return emb_read_file_subanswer_data(iterator_, _i);
}

bool read_file_t::answer_iterator_t::operator == (const answer_iterator_t& _a) const {
    return (this->iterator_ == _a.iterator_);
}

bool read_file_t::answer_iterator_t::operator != (const answer_iterator_t& _a) const {
    return !operator == (_a);
}

void read_file_t::answer_iterator_t::operator ++ () {
    iterator_ = emb_read_file_next_subanswer(ans, iterator_);
}

void read_file_t::answer_iterator_t::operator ++ (int) {
    iterator_ = emb_read_file_next_subanswer(ans, iterator_);
}

void read_file_t::answer_iterator_t::reset_to_begin() {
    if(ans)
        iterator_ = emb_read_file_next_subanswer(ans, NULL);
}

read_file_t::read_file_t() { }
read_file_t::read_file_t(transaction_t& _tr) : transact_base_t(_tr) { }

void read_file_t::build_req(const req_t &_reqs) {
    int res;
    tr->req.resize(emb_read_file_calc_req_data_size(_reqs.size()));
    tr->ans.resize(emb_read_file_calc_answer_data_size(&_reqs[0], _reqs.size()));

    if(res = emb_read_file_make_req(tr->req, &_reqs[0], _reqs.size()))
        throw res;
}

read_file_t::answer_iterator_t read_file_t::subanswer_begin() const {
    return answer_iterator_t(tr->ans);
}

read_file_t::answer_iterator_t read_file_t::subanswer_end() const {
    return answer_iterator_t(NULL);
}

read_file_t::answer_iterator_t read_file_t::operator [] (int _subanswer_index) const {
    emb_read_file_subansw_t* i = emb_read_file_find_subanswer(tr->ans, _subanswer_index);
    answer_iterator_t res(tr->ans);
    if(i)
        res.iterator_ = i;
    else
        res.ans = NULL;
    return res;
}

read_file_t::answer_t& read_file_t::get_answer(answer_t& _res) {
    for(answer_iterator_t ii=subanswer_begin(); ii != subanswer_end(); ++ii) {
        _res.push_back(subanswer_t());
        const uint16_t length = ii.subanswer_quantity();
        _res.back().length = length;
        for(uint16_t j=0; j<length; ++j)
            _res.back().data.push_back(ii.subanswer_data(j));
    }
    return _res;
}

// *******************************************************************************
// write_file_t

write_file_t::subreq_t::subreq_t(uint16_t _file_number,
                                 uint16_t _record_number,
                                 const regs_t& _regs)

                 : file_number(_file_number)
                 , record_number(_record_number)
                 , regs(_regs) { }

write_file_t::write_file_t() { }
write_file_t::write_file_t(transaction_t& _tr) : transact_base_t(_tr) { }

void write_file_t::build_req(const req_t& _req) {
    int res;
    std::vector<emb_write_file_req_t> reqs(_req.size());
    for(size_t i=0; i<_req.size(); ++i) {
        reqs[i].file_number = _req[i].file_number;
        reqs[i].record_number = _req[i].record_number;
        reqs[i].record_length = _req[i].regs.size();
        reqs[i].data = &_req[i].regs[0];
    }

    tr->req.resize(emb_write_file_calc_req_data_size(&reqs[0], reqs.size()));
    tr->ans.resize(emb_write_file_calc_answer_data_size(&reqs[0], reqs.size()));

    if(res = emb_write_file_make_req(tr->req, &reqs[0], reqs.size()))
        throw res;
}

// *******************************************************************************
// read_fifo_t

read_fifo_t::read_fifo_t() { }
read_fifo_t::read_fifo_t(transaction_t& _tr) : transact_base_t(_tr) { }

void read_fifo_t::build_req(uint16_t _starting_address) {
    int res;
    tr->req.resize(emb_read_fifo_calc_req_data_size());
    tr->ans.resize(emb_read_fifo_calc_answer_data_size());

    if((res = emb_read_fifo_make_req(tr->req, _starting_address)))
        throw res;
}

uint16_t read_fifo_t::get_answer_regs_count() const {
    return emb_read_fifo_regs_count(tr->ans);
}

uint16_t read_fifo_t::get_answer_data(uint16_t _offset) const {
    return emb_read_fifo_get_data(tr->ans, _offset);
}

uint16_t read_fifo_t::get_answer_all_data(uint16_t _buf_size, uint16_t* _buf) const {
    return emb_read_fifo_get_all_data(tr->ans, _buf_size, _buf);
}

regs_t& read_fifo_t::get_answer_all_data(regs_t& _result) const {
    const uint16_t sz = get_answer_regs_count();
    _result.resize(sz);
    emb_read_fifo_get_all_data(tr->ans, sz, &_result[0]);
    return _result;
}

// *******************************************************************************
// client_t

client_t::client_t()
    : is_sync(false)
    , curr_transaction(NULL)
{
    memset(&client, 0, sizeof(struct emb_client_t));
    client.on_response = emb_on_response_;
    client.on_error = emb_on_error_;
    emb_client_init(&client);
}

int client_t::do_transaction(int _server_addr,
                             unsigned int _timeout,
                             transaction_t &_transaction)
{
    int res;

    curr_transaction = &_transaction;
    curr_server_addr = _server_addr;

    if((res = emb_client_do_transaction(&client, _server_addr, _transaction)) != 0) {
        return res;
    }

    if(is_sync) {
        res = emb_sync_client_start_wait(_timeout);

        if(!res) {
            return result;
        }
        else if(res == 1) {
            return -modbus_resp_timeout;
        }
        else {
            return res;
        }
    }
    else {
        return res;
    }
}

void client_t::set_transport(struct emb_transport_t* _transport)
{
    emb_client_set_transport(&client, _transport);
}

void client_t::set_sync(bool _is_sync) { is_sync = _is_sync; }

bool client_t::get_sync() const { return is_sync; }

void client_t::emb_on_response(int _slave_addr) { }

void client_t::emb_on_error(int _slave_addr, int _errno) { }

int client_t::emb_sync_client_start_wait(unsigned int _timeout) { return 0; }

void client_t::sync_answer_timeout()
{
    emb_client_wait_timeout(&client);
}

void client_t::emb_on_response_(struct emb_client_t* _req, int _slave_addr)
{
    client_t* _this = container_of(_req, client_t, client);
    _this->result = 0;
    _this->emb_on_response(_slave_addr);
}

void client_t::emb_on_error_(struct emb_client_t* _req, int _slave_addr, int _errno)
{
    client_t* _this = container_of(_req, client_t, client);
    _this->result = _errno;
    _this->emb_on_error(_slave_addr, _errno);
}

// *******************************************************************************
// proxy_t

proxy_t::holdings_t::reg_t::operator int() const
{
    read_regs_t r(p->transaction);
    r.build_req(EMB_RR_HOLDINGS, addr, 1);
    p->do_transaction(p->transaction);
    return r.get_answer_reg(0);
}

void proxy_t::holdings_t::reg_t::operator = (int _v)
{
    write_reg_t r(p->transaction);
    r.build_req(addr, _v);
    p->do_transaction(p->transaction);
}

void proxy_t::holdings_t::reg_t::operator |= (int _v)
{
    write_mask_reg_t r(p->transaction);
    r.build_req(addr, ~_v, _v);
    p->do_transaction(p->transaction);
}

void proxy_t::holdings_t::reg_t::operator &= (int _v)
{
    write_mask_reg_t r(p->transaction);
    r.build_req(addr, ~_v, 0);
    p->do_transaction(p->transaction);
}

void proxy_t::holdings_t::reg_t::operator = (const emb::regs_t& _regs)
{
    write_regs_t r(p->transaction);
    r.build_req(addr, _regs.size(), &_regs[0]);
    p->do_transaction(p->transaction);
}

proxy_t::holdings_t::reg_t::operator float() const
{
    read_regs_t r(p->transaction);
    float res;
    r.build_req(EMB_RR_HOLDINGS, addr, 2);
    p->do_transaction(p->transaction);
    r.get_answer_regs((uint16_t*)&res, 0, r.get_answer_quantity());
    return res;
}

void proxy_t::holdings_t::reg_t::operator = (float _v)
{
    write_regs_t r(p->transaction);
    r.build_req(addr, 2, (uint16_t*)&_v);
    p->do_transaction(p->transaction);
}

void proxy_t::holdings_t::reg_t::set_bit(unsigned char _nbit, bool _value)
{
    write_mask_reg_t r(p->transaction);
    const uint16_t mask = (1 << _nbit);
    r.build_req(addr, ~mask, _value ? mask : 0);
    p->do_transaction(p->transaction);
}

bool proxy_t::holdings_t::reg_t::get_bit(unsigned char _nbit)
{
    read_regs_t r(p->transaction);
    uint16_t res;
    r.build_req(EMB_RR_HOLDINGS, addr, 1);
    p->do_transaction(p->transaction);
    r.get_answer_regs((uint16_t*)&res, 0, 1);
    return (res & (1 << _nbit)) != 0;
}

proxy_t::holdings_t::regs_t::operator emb::regs_t()
{
    read_regs_t r(p->transaction);
    emb::regs_t res;
    r.build_req(EMB_RR_HOLDINGS, start, (end - start)+1);
    p->do_transaction(p->transaction);
    r.get_answer_regs(res, 0, r.get_answer_quantity());
    return res;
}

void proxy_t::holdings_t::regs_t::operator = (const emb::regs_t& _regs)
{
    write_regs_t r(p->transaction);
    r.build_req(start, _regs.size(), &_regs[0]);
    p->do_transaction(p->transaction);
}

proxy_t::holdings_t::reg_t& proxy_t::holdings_t::operator[] (uint16_t _i)
{
    reg.p = p;
    reg.addr = _i;
    return reg;
}

proxy_t::holdings_t::regs_t& proxy_t::holdings_t::operator[] (const emb::range_t& _r)
{
    regs.p = p;
    regs.start = _r.first;
    regs.end = _r.second;
    return regs;
}

proxy_t::proxy_t()
    : client_(NULL)
    , server_addr_(0)
    , timeout_(DEFAULT_TIMEOUT)
{
    holdings.p = this;
}

proxy_t::proxy_t(client_t *_client, int _server_addr)
    : client_(_client)
    , server_addr_(_server_addr)
    , timeout_(DEFAULT_TIMEOUT)
{
    holdings.p = this;
}

void proxy_t::set_client(client_t* _client)
{ client_ = _client; }

client_t* proxy_t::client() const
{ return client_; }

void proxy_t::set_server_address(int _server_address)
{ server_addr_ = _server_address; }

int proxy_t::server_address() const
{ return server_addr_; }

void proxy_t::set_timeout(unsigned int _time)
{ timeout_ = _time; }

unsigned int proxy_t::timeout() const
{ return timeout_; }

// Discrete inputs
void proxy_t::read_discrete_inputs(uint16_t _begin, uint16_t _size, inputs_t& _result)
{
    read_discrete_inputs_t r(transaction);
    r.build_req(_begin, _size);
    do_transaction(transaction);
    _result.resize(r.get_req_quantity());
    for(int i=0; i<r.get_req_quantity(); ++i)
        _result[i] = r.get_answer_bit(i);
}

// Coils
void proxy_t::read_coils(uint16_t _begin, uint16_t _size, coils_t& _result)
{
    read_coils_t r(transaction);
    r.build_req(_begin, _size);
    do_transaction(transaction);
    _result.resize(r.get_req_quantity());
    for(int i=0; i<r.get_req_quantity(); ++i)
        _result[i] = r.get_answer_bit(i);
}

void proxy_t::write_coil(uint16_t _addr, bool _value)
{
    write_coil_t r(transaction);
    r.build_req(_addr, _value);
    do_transaction(transaction);
}

//void proxy_t::write_coils(uint16_t _begin, const coils_t& _values)
//{
//    write_coils_t r(transaction);
//    r.build_req(_begin, _values.size(), _values.data());
//    do_transaction(transaction);
//}

// Input registers
void proxy_t::read_input_regs(uint16_t _begin, uint16_t _size, regs_t& _result)
{
    read_input_regs_t r(transaction);
    r.build_req(_begin, _size);
    do_transaction(transaction);
    r.get_answer_regs(_result, 0, r.get_answer_quantity());
}

// Holding registers
void proxy_t::read_holdings(uint16_t _begin, uint16_t _size, regs_t& _result)
{
    read_holding_regs_t r(transaction);
    r.build_req(_begin, _size);
    do_transaction(transaction);
    r.get_answer_regs(_result, 0, r.get_answer_quantity());
}

void proxy_t::write_holding(uint16_t _addr, uint16_t _value)
{
    write_reg_t r(transaction);
    r.build_req(_addr, _value);
    do_transaction(transaction);
}

void proxy_t::write_holdings(uint16_t _begin, const regs_t& _values)
{
    write_regs_t r(transaction);
    r.build_req(_begin, _values.size(), &_values[0]);
    do_transaction(transaction);
}

void proxy_t::read_write_holdings(uint16_t _read_begin, uint16_t _read_size, regs_t& _read_result,
                                  uint16_t _write_begin, const regs_t& _to_write)
{
    read_write_regs_t r(transaction);
    r.build_req(_read_begin, _read_size, _write_begin, _to_write.size(), _to_write.data());
    do_transaction(transaction);
    r.get_answer_rd_regs(_read_result, 0, r.get_answer_rd_quantity());
}

// File records
void proxy_t::read_file(const read_file_t::req_t& _req, read_file_t::answer_t& _result)
{
    read_file_t r(transaction);
    r.build_req(_req);
    do_transaction(transaction);
    r.get_answer(_result);
}

void proxy_t::write_file(const write_file_t::req_t& _req)
{
    write_file_t r(transaction);
    r.build_req(_req);
    do_transaction(transaction);
}

// FIFO
void proxy_t::read_fifo(uint16_t _begin, regs_t& _result)
{
    read_fifo_t r;
    r.build_req(_begin);
    r.get_answer_all_data(_result);
}

void proxy_t::do_transaction(transaction_t& _tr)
{
    const int res = client_->do_transaction(server_addr_, timeout_, _tr);
    if(res)
        throw res;
}

} // namespace client

namespace server {

// *******************************************************************************
//  #####  #####  ####   #   #  #####  ####
//  #      #      #   #  #   #  #      #   #
//  #####  #####  ####   #   #  #####  ####
//      #  #      #   #   # #   #      #   #
//  #####  #####  #   #    #    #####  #   #

// *******************************************************************************
// server_coils_t

coils_t::coils_t()
{
    set_funcs();
}

coils_t::coils_t(uint16_t _start, uint16_t _size)
{
    coils.start = _start;
    coils.size = _size;
    set_funcs();
}

void coils_t::set_start(uint16_t _start)
{ coils.start = _start; }

void coils_t::set_size(uint16_t _size)
{ coils.size = _size; }

uint8_t coils_t::on_read_coils(uint16_t _offset,
                             uint16_t _quantity,
                             uint8_t* _pvalues)
{ return 0; }

uint8_t coils_t::on_write_coils(uint16_t _offset,
                              uint16_t _quantity,
                              const uint8_t* _pvalues)
{ return 0; }

uint16_t coils_t::start() const { return coils.start; }
uint16_t coils_t::size() const { return coils.size; }

void coils_t::set_funcs()
{
    coils.read_bits = read_coils;
    coils.write_bits = write_coils;
}

uint8_t coils_t::read_coils(struct emb_srv_bits_t* _coils,
                                     uint16_t _offset,
                                     uint16_t _quantity,
                                     uint8_t* _pvalues)
{
    coils_t* _this = container_of(_coils, coils_t, coils);
    return _this->on_read_coils(_offset, _quantity, _pvalues);
}

uint8_t coils_t::write_coils(struct emb_srv_bits_t* _coils,
                                      uint16_t _offset,
                                      uint16_t _quantity,
                                      const uint8_t* _pvalues)
{
    coils_t* _this = container_of(_coils, coils_t, coils);
    return _this->on_write_coils(_offset, _quantity, _pvalues);
}

// *******************************************************************************
// discrete_inputs_t

discrete_inputs_t::discrete_inputs_t()
{
    set_funcs();
}

discrete_inputs_t::discrete_inputs_t(uint16_t _start, uint16_t _size)
{
    discrete_inputs.start = _start;
    discrete_inputs.size = _size;
    set_funcs();
}

void discrete_inputs_t::set_start(uint16_t _start)
{ discrete_inputs.start = _start; }

void discrete_inputs_t::set_size(uint16_t _size)
{ discrete_inputs.size = _size; }

uint8_t discrete_inputs_t::on_read_bits(uint16_t _offset,
                             uint16_t _quantity,
                             uint8_t* _pvalues)
{ return 0; }

uint16_t discrete_inputs_t::start() const { return discrete_inputs.start; }
uint16_t discrete_inputs_t::size() const { return discrete_inputs.size; }

void discrete_inputs_t::set_funcs()
{
    discrete_inputs.read_bits = read_inputs;
    discrete_inputs.write_bits = NULL;
}

uint8_t discrete_inputs_t::read_inputs(struct emb_srv_bits_t* _bits,
                                     uint16_t _offset,
                                     uint16_t _quantity,
                                     uint8_t* _pvalues)
{
    discrete_inputs_t* _this = container_of(_bits, discrete_inputs_t, discrete_inputs);
    return _this->on_read_bits(_offset, _quantity, _pvalues);
}

// *******************************************************************************
// input_regs_t

input_regs_t::input_regs_t()
{
    set_funcs();
}

input_regs_t::input_regs_t(uint16_t _start, uint16_t _size)
{
    h.start = _start;
    h.size = _size;
    set_funcs();
}

void input_regs_t::set_start(uint16_t _start)
{ h.start = _start; }

void input_regs_t::set_size(uint16_t _size)
{ h.size = _size; }

uint8_t input_regs_t::on_read_regs(uint16_t _offset,
                             uint16_t _quantity,
                             uint16_t* _pvalues)
{ return 0; }

uint16_t input_regs_t::start() const { return h.start; }
uint16_t input_regs_t::size() const { return h.size; }

void input_regs_t::set_funcs()
{
    h.read_regs = read_regs;
    h.write_regs = NULL;
}

uint8_t input_regs_t::read_regs(struct emb_srv_regs_t* _rr,
                                     uint16_t _offset,
                                     uint16_t _quantity,
                                     uint16_t* _pvalues)
{
    input_regs_t* _this = container_of(_rr, input_regs_t, h);
    return _this->on_read_regs(_offset, _quantity, _pvalues);
}

// *******************************************************************************
// holding_regs_t

holding_regs_t::holding_regs_t()
{
    set_funcs();
}

holding_regs_t::holding_regs_t(uint16_t _start, uint16_t _size)
{
    h.start = _start;
    h.size = _size;
    set_funcs();
}

void holding_regs_t::set_start(uint16_t _start)
{ h.start = _start; }

void holding_regs_t::set_size(uint16_t _size)
{ h.size = _size; }

uint8_t holding_regs_t::on_read_regs(uint16_t _offset,
                             uint16_t _quantity,
                             uint16_t* _pvalues)
{ return 0; }

uint8_t holding_regs_t::on_write_regs(uint16_t _offset,
                              uint16_t _quantity,
                              const uint16_t* _pvalues)
{ return 0; }

uint16_t holding_regs_t::start() const { return h.start; }
uint16_t holding_regs_t::size() const { return h.size; }

void holding_regs_t::set_funcs()
{
    h.read_regs = read_regs;
    h.write_regs = write_regs;
}

uint8_t holding_regs_t::read_regs(struct emb_srv_regs_t* _rr,
                                     uint16_t _offset,
                                     uint16_t _quantity,
                                     uint16_t* _pvalues)
{
    holding_regs_t* _this = container_of(_rr, holding_regs_t, h);
    return _this->on_read_regs(_offset, _quantity, _pvalues);
}

uint8_t holding_regs_t::write_regs(struct emb_srv_regs_t* _rr,
                                      uint16_t _offset,
                                      uint16_t _quantity,
                                      const uint16_t* _pvalues)
{
    holding_regs_t* _this = container_of(_rr, holding_regs_t, h);
    return _this->on_write_regs(_offset, _quantity, _pvalues);
}

// *******************************************************************************
// file_record_t

file_record_t::file_record_t()
{
    set_funcs();
}

file_record_t::file_record_t(uint16_t _fileno/*, uint16_t _start, uint16_t _size*/)
{
    f.fileno = _fileno;
//    f.start = _start;
//    f.size = _size;
    set_funcs();
}

void file_record_t::set_fileno(uint16_t _fileno)
{ f.fileno = _fileno; }

//void server_file_t::set_start(uint16_t _start)
//{ f.start = _start; }

//void server_file_t::set_size(uint16_t _size)
//{ f.size = _size; }

uint8_t file_record_t::on_read_file(uint16_t _offset,
                             uint16_t _quantity,
                             uint16_t* _pvalues)
{ return 0; }

uint8_t file_record_t::on_write_file(uint16_t _offset,
                              uint16_t _quantity,
                              const uint16_t* _pvalues)
{ return 0; }

void file_record_t::set_funcs()
{
    f.read_file = read_file;
    f.write_file = write_file;
}

uint8_t file_record_t::read_file(emb_srv_file_t *_f,
                                 uint16_t _offset,
                                 uint16_t _quantity,
                                 uint16_t* _pvalues)
{
    file_record_t* _this = container_of(_f, file_record_t, f);
    return _this->on_read_file(_offset, _quantity, _pvalues);
}

uint8_t file_record_t::write_file(emb_srv_file_t *_f,
                                  uint16_t _offset,
                                  uint16_t _quantity,
                                  const uint16_t* _pvalues)
{
    file_record_t* _this = container_of(_f, file_record_t, f);
    return _this->on_write_file(_offset, _quantity, _pvalues);
}

// *******************************************************************************
// server_t

server_t::server_t(int _address) : address(_address)
{
    memset(&srv, 0, sizeof(struct emb_server_t));
    srv.get_function = get_function;
    srv.get_coils = get_coils;
    srv.get_discrete_inputs = get_discrete_inputs;
    srv.get_input_regs = get_input_regs;
    srv.get_holding_regs = get_holding_regs;
    srv.get_file = get_file;
    srv.read_fifo = read_fifo;
    srv.flags = 0;
    functions.resize(MAX_FUNCTION_NUMBER+1, NULL);
}

int server_t::addr() const { return address; }

bool server_t::add_function(uint8_t _func_no, emb_srv_function_t _func)
{
    if(functions.size() > _func_no) {
        functions[_func_no] = _func;
        return true;
    }
    return false;
}

#define IS_POINT_IN_RANGE(_range_start_, _range_end_, _point_) \
    (((_range_start_) < (_point_)) && ((_point_) < (_range_end_)))

static bool emb_is_ranges_cross(uint16_t _start1,
                         uint16_t _size1,
                         uint16_t _start2,
                         uint16_t _size2)
{
    const uint32_t end1 = _start1 + _size1;
    const uint32_t end2 = _start2 + _size2;

    if( IS_POINT_IN_RANGE(_start1, end1, _start2) ||
        IS_POINT_IN_RANGE(_start1, end1, end2) ||
        IS_POINT_IN_RANGE(_start2, end2, _start1) ||
        IS_POINT_IN_RANGE(_start2, end2, end1))
        return true;

    return false;
}

bool server_t::add_coils(coils_t& _coils)
{
    for(coils_iter i=coils.begin(); i != coils.end(); ++i) {
        if( emb_is_ranges_cross((*i)->coils.start, (*i)->coils.size, _coils.coils.start, _coils.coils.size) )
            return false;
    }
    coils.push_back(&_coils);
    return true;
}

bool server_t::add_discrete_inputs(discrete_inputs_t& _inputs)
{
    for(discr_inputs_iter i=discrete_inputs.begin(); i != discrete_inputs.end(); ++i) {
        if( emb_is_ranges_cross((*i)->discrete_inputs.start, (*i)->discrete_inputs.size,
                                _inputs.discrete_inputs.start, _inputs.discrete_inputs.size) )
            return false;
    }
    discrete_inputs.push_back(&_inputs);
    return true;
}

bool server_t::add_holding_regs(holding_regs_t& _holdings)
{
    for(holdnigs_iter i=holdings.begin(); i != holdings.end(); ++i) {
        if( emb_is_ranges_cross((*i)->h.start, (*i)->h.size, _holdings.h.start, _holdings.h.size) )
            return false;
    }
    holdings.push_back(&_holdings);
    return true;
}

bool server_t::add_input_regs(input_regs_t& _holdings)
{
    for(input_regs_iter i=input_regs.begin(); i != input_regs.end(); ++i) {
        if( emb_is_ranges_cross((*i)->h.start, (*i)->h.size, _holdings.h.start, _holdings.h.size) )
            return false;
    }
    input_regs.push_back(&_holdings);
    return true;
}

bool server_t::add_file(file_record_t& _file)
{
    for(files_iter i=files.begin(); i != files.end(); ++i) {
        if((*i)->f.fileno == _file.f.fileno)
            //if(emb_is_ranges_cross((*i)->f.start, (*i)->f.size, _file.f.start, _file.f.size) )
                return false;
    }
    files.push_back(&_file);
    return true;
}

uint8_t server_t::on_read_fifo(uint16_t _address,
                               uint16_t* _fifo_buf,
                               uint8_t* _fifo_count)
{
    *_fifo_count = 0;
    return 0; // May be I need to return an MBE_ILLEGAL_FUNCTION here ?
}

emb_srv_function_t server_t::get_function(struct emb_server_t* _srv, uint8_t _func)
{
    server_t* _this = container_of(_srv, server_t, srv);
    if(_this->functions.size() > _func)
        return _this->functions[_func];
    return NULL;
}

struct emb_srv_bits_t* server_t::get_coils(struct emb_server_t* _srv, uint16_t _begin)
{
    server_t* _this = container_of(_srv, server_t, srv);
    for(coils_iter i=_this->coils.begin(); i != _this->coils.end(); ++i) {
        const uint16_t start = (*i)->coils.start;
        const uint16_t end = start + (*i)->coils.size - 1;
        if((start <= _begin) && (_begin <= end))
            return &((*i)->coils);
    }
    return NULL;
}

struct emb_srv_bits_t* server_t::get_discrete_inputs(struct emb_server_t* _srv, uint16_t _begin)
{
    server_t* _this = container_of(_srv, server_t, srv);
    for(discr_inputs_iter i=_this->discrete_inputs.begin(); i != _this->discrete_inputs.end(); ++i) {
        const uint16_t start = (*i)->discrete_inputs.start;
        const uint16_t end = start + (*i)->discrete_inputs.size - 1;
        if((start <= _begin) && (_begin <= end))
            return &((*i)->discrete_inputs);
    }
    return NULL;
}

struct emb_srv_regs_t* server_t::get_holding_regs(struct emb_server_t* _srv, uint16_t _begin)
{
    server_t* _this = container_of(_srv, server_t, srv);
    for(holdnigs_iter i=_this->holdings.begin(); i != _this->holdings.end(); ++i) {
        const uint16_t start = (*i)->h.start;
        const uint16_t end = start + (*i)->h.size - 1;
        if((start <= _begin) && (_begin <= end))
            return &((*i)->h);
    }
    return NULL;
}

struct emb_srv_regs_t* server_t::get_input_regs(struct emb_server_t* _srv, uint16_t _begin)
{
    server_t* _this = container_of(_srv, server_t, srv);
    for(input_regs_iter i=_this->input_regs.begin(); i != _this->input_regs.end(); ++i) {
        const uint16_t start = (*i)->h.start;
        const uint16_t end = start + (*i)->h.size - 1;
        if((start <= _begin) && (_begin <= end))
            return &((*i)->h);
    }
    return NULL;
}

struct emb_srv_file_t* server_t::get_file(struct emb_server_t* _srv, uint16_t _fileno/*, uint16_t _begin*/)
{
    server_t* _this = container_of(_srv, server_t, srv);
    for(files_iter i=_this->files.begin(); i != _this->files.end(); ++i) {
        if((*i)->f.fileno == _fileno)
            return &((*i)->f);
    }
    return NULL;
}

uint8_t server_t::read_fifo(struct emb_server_t* _srv, uint16_t _address,
                            uint16_t* _fifo_buf, uint8_t* _fifo_count)
{
    server_t* _this = container_of(_srv, server_t, srv);
    return _this->on_read_fifo(_address, _fifo_buf, _fifo_count);
}

// *******************************************************************************
// super_server_t

super_server_t::super_server_t()
{
    memset(&ssrv, 0, sizeof(struct emb_super_server_t));

    ssrv.get_server = _get_server;

    rx_pdu.resize(MAX_PDU_DATA_SIZE);
    tx_pdu.resize(MAX_PDU_DATA_SIZE);

    ssrv.rx_pdu = &rx_pdu;
    ssrv.tx_pdu = &tx_pdu;
}

void super_server_t::set_transport(struct emb_transport_t* _transport)
{
    emb_super_server_set_transport(&ssrv, _transport);
}

bool super_server_t::add_server(server_t& _srv)
{
    for(srv_iter i=servers.begin(); i != servers.end(); ++i) {
        if((*i)->addr() == _srv.addr())
            return false;
    }
    servers.push_back(&_srv);
    return true;
}

struct emb_server_t* super_server_t::_get_server(struct emb_super_server_t* _ssrv,
                                        uint8_t _address)
{
    super_server_t* _this = container_of(_ssrv, super_server_t, ssrv);
    for(srv_iter i=_this->servers.begin(); i != _this->servers.end(); ++i) {
        if((*i)->addr() == _address)
            return &(*i)->srv;
    }
    return NULL;
}

} // namespace server

} // namespace emb
