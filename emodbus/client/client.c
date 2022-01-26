
#include <emodbus/client/client.h>
#include <emodbus/base/modbus_errno.h>
#include <emodbus/client/read_bits.h>
#include <emodbus/client/read_fifo.h>
#include <emodbus/client/read_file_record.h>
#include <emodbus/client/read_regs.h>
#include <emodbus/client/read_write_regs.h>
#include <emodbus/client/write_coil.h>
#include <emodbus/client/write_coils.h>
#include <emodbus/client/write_file_record.h>
#include <emodbus/client/write_mask_reg.h>
#include <emodbus/client/write_multi_regs.h>
#include <emodbus/client/write_single_reg.h>
#include <emodbus/base/limits.h>
#include <emodbus/base/bit_array.h>
#include <string.h>
#include <errno.h>

/*!
 * \file
 * \brief Realisation of modbus client (master) side.
 *
 */

void emb_sync_client_init(emb_sync_client_t* _cli)
{
    if (_cli) {
        _cli->bad_transactions = 0;
        _cli->good_transactions = 0;
        //_cli->transaction_id_counter = 0;
    }
}

int emb_sync_client_do_request(emb_sync_client_t* _cli, emb_adu_t* _req_adu, emb_adu_t* _ans_adu)
{
    int r = 0;
    unsigned int retries = 0;

    //_cli->transaction_id_counter++;

    while(_cli->n_retries >= retries) {

//        _req_adu->transaction_id = _cli->transaction_id_counter;

        if(!(_cli && _cli->recv_adu && _cli->send_adu && _req_adu && _ans_adu)) {
            return -modbus_invalid_argument;
        }

        r = _cli->send_adu(_cli, _req_adu);
        if(r != modbus_success) {
            _cli->bad_transactions++;
            retries++;
            continue;
        }

        r = _cli->recv_adu(_cli, _ans_adu);
        if(r != modbus_success) {
            _cli->bad_transactions++;
            retries++;
            continue;
        }

        if(_req_adu->server_id != _ans_adu->server_id)
            return -modbus_resp_wrong_address;

        r = emb_check_pdu_for_exception(&_ans_adu->pdu);
        if(r != 0) {
            _cli->bad_transactions++;
            retries++;
            continue;
        }

//        if (_ans_adu->transaction_id != _cli->transaction_id_counter) {
//            _cli->bad_transactions++;
//            retries++;
//            r = -modbus_resp_wrong_transaction_id;
//            continue;
//        }

        _cli->good_transactions++;
        break;
    }

    return r;
}

static int __emb_sync_client_read_bits(emb_sync_client_t* _cli,
                                       enum EMB_RB_TYPE _rb_type,
                                       uint16_t _start_address,
                                       uint16_t _quantity,
                                       uint8_t* _result,
                                       uint32_t _bit_offset)
{
    int res;
    int n_bytes;
    const uint8_t* p_answ_data;

    res = emb_read_bits_make_req(&_cli->req_adu->pdu, _rb_type, _start_address, _quantity);
    if(res != modbus_success)
        return res;

    res = emb_sync_client_do_request(_cli, _cli->req_adu, _cli->ans_adu);
    if(res != modbus_success)
        return res;

    n_bytes = emb_read_bits_get_answ_bytes_count(MB_CONST_PDU(&_cli->ans_adu->pdu));
    if(n_bytes < 0)
        return res;

    if((n_bytes * 8) < _quantity)
        return -modbus_wrong_resp_quantity;

    res = emb_read_bits_get_answ_data(MB_CONST_PDU(&_cli->ans_adu->pdu), &p_answ_data);
    if (res != 0)
        return res;

    res = emb_bit_arr_set_bits((emb_ba_word_t*)_result, 65536,
                               (const emb_ba_word_t*)p_answ_data, _bit_offset, _quantity);

    return res;
}

int emb_sync_client_read_bits(emb_sync_client_t* _cli,
                              uint8_t _server_id,
                              enum EMB_RB_TYPE _rb_type,
                              uint16_t _start_address,
                              uint32_t _quantity,
                              uint8_t* _result)
{
    uint32_t counter = 0;

    if (!(_cli && _cli->req_adu && _cli->ans_adu && _quantity && _result))
        return -modbus_invalid_argument;

    if ((uint32_t)_start_address + _quantity > 65536)
        return -modbus_invalid_argument;

    _cli->req_adu->server_id = _server_id;

    while (counter < _quantity) {
        uint32_t q = _quantity - counter;
        if (q > EMB_READ_BITS_MAX_QUANTITY)
            q = EMB_READ_BITS_MAX_QUANTITY;
        int res = __emb_sync_client_read_bits(_cli, _rb_type, _start_address + (uint16_t)counter, (uint16_t)q, _result, counter);
        if (res != modbus_success)
            return res;
        counter += q;
    }
    return modbus_success;
}

static int __emb_sync_client_read_regs(emb_sync_client_t* _cli,
                                       enum EMB_RR_TYPE _rr_type,
                                       uint16_t _start_address,
                                       uint16_t _quantity,
                                       uint16_t* _result)
{
    int res;

    res = emb_read_regs_make_req(&_cli->req_adu->pdu, _rr_type, _start_address, _quantity);
    if(res != modbus_success)
        return res;

    res = emb_sync_client_do_request(_cli, _cli->req_adu, _cli->ans_adu);
    if(res != modbus_success)
        return res;

    int regsn = emb_read_regs_get_ans_regs_n(MB_CONST_PDU(&_cli->ans_adu->pdu));
    if(regsn < 0)
        return res;
    else if(regsn != _quantity)
        return -modbus_wrong_resp_quantity;

    return emb_read_regs_get_ans_regs(MB_CONST_PDU(&_cli->ans_adu->pdu), 0, (uint16_t)regsn, _result);
}

int emb_sync_client_read_regs(emb_sync_client_t* _cli,
                              uint8_t _server_id,
                              enum EMB_RR_TYPE _rr_type,
                              uint16_t _start_address,
                              uint32_t _quantity,
                              uint16_t* _result)
{
    uint32_t counter = 0;

    if(!(_cli && _cli->req_adu && _cli->ans_adu && _quantity && _result))
        return -modbus_invalid_argument;

    if((uint32_t)_start_address + _quantity > 65536)
        return -modbus_invalid_argument;

    _cli->req_adu->server_id = _server_id;

    while(counter < _quantity) {
        uint32_t q = _quantity - counter;
        if(q > EMB_READ_REGS_MAX_QUANTITY)
            q = EMB_READ_REGS_MAX_QUANTITY;
        int res = __emb_sync_client_read_regs(_cli, _rr_type, _start_address + (uint16_t)counter, (uint16_t)q, _result + counter);
        if(res != modbus_success)
            return res;
        counter += q;
    }
    return modbus_success;
}

int emb_sync_client_write_coil(emb_sync_client_t* _cli,
                               uint8_t _server_id,
                               uint16_t _address,
                               char _value)
{
    int res;

    if(!(_cli && _cli->req_adu && _cli->ans_adu))
        return -modbus_invalid_argument;

    _cli->req_adu->server_id = _server_id;

    res = emb_write_coil_make_req(&_cli->req_adu->pdu, _address, _value);
    if(res != modbus_success)
        return res;

    return emb_sync_client_do_request(_cli, _cli->req_adu, _cli->ans_adu);
}

static int __emb_sync_client_write_coils(emb_sync_client_t* _cli,
                                         uint16_t _start_address,
                                         uint16_t _quantity,
                                         const uint8_t* _values)
{
    int res;

    res = emb_write_coils_make_req(&_cli->req_adu->pdu, _start_address, _quantity, _values);
    if(res != modbus_success)
        return res;

    return emb_sync_client_do_request(_cli, _cli->req_adu, _cli->ans_adu);
}

int emb_sync_client_write_coils(emb_sync_client_t* _cli,
                                uint8_t _server_id,
                                uint16_t _start_address,
                                uint32_t _quantity,
                                const uint8_t* _values)
{
    uint32_t counter = 0;

    if(!(_cli && _cli->req_adu && _cli->ans_adu && _quantity && _values))
        return -modbus_invalid_argument;

    if((uint32_t)_start_address + _quantity > 65536)
        return -modbus_invalid_argument;

    _cli->req_adu->server_id = _server_id;

    while(counter < _quantity) {
        uint32_t q = _quantity - counter;
        if(q > EMB_WRITE_COILS_MAX_QUANTITY)
            q = EMB_WRITE_COILS_MAX_QUANTITY;
        int res = __emb_sync_client_write_coils(_cli, _start_address + (uint16_t)counter, (uint16_t)q, _values + counter / 8);
        if(res != modbus_success)
            return res;
        counter += q;
    }
    return modbus_success;
}

int emb_sync_client_mask_reg(emb_sync_client_t* _cli,
                             uint8_t _server_id,
                             uint16_t _address,
                             uint16_t _and_mask,
                             uint16_t _or_mask)
{
    int res;

    if(!(_cli && _cli->req_adu && _cli->ans_adu))
        return -modbus_invalid_argument;

    _cli->req_adu->server_id = _server_id;

    res = emb_write_mask_reg_make_req(&_cli->req_adu->pdu, _address, _and_mask, _or_mask);
    if(res != modbus_success)
        return res;

    return emb_sync_client_do_request(_cli, _cli->req_adu, _cli->ans_adu);
}

int emb_sync_client_write_reg(emb_sync_client_t* _cli,
                              uint8_t _server_id,
                              uint16_t _address,
                              uint16_t _value)
{
    int res;

    if(!(_cli && _cli->req_adu && _cli->ans_adu))
        return -modbus_invalid_argument;

    _cli->req_adu->server_id = _server_id;

    res = emb_write_reg_make_req(&_cli->req_adu->pdu, _address, _value);
    if(res != modbus_success)
        return res;

    return emb_sync_client_do_request(_cli, _cli->req_adu, _cli->ans_adu);
}



static int __emb_sync_client_write_regs(emb_sync_client_t* _cli,
                                        uint16_t _start_address,
                                        uint16_t _quantity,
                                        const uint16_t* _values)
{
    int res;

    res = emb_write_regs_make_req(&_cli->req_adu->pdu, _start_address, _quantity, _values);
    if(res != modbus_success)
        return res;

    return emb_sync_client_do_request(_cli, _cli->req_adu, _cli->ans_adu);
}

int emb_sync_client_write_regs(emb_sync_client_t* _cli,
                               uint8_t _server_id,
                               uint16_t _start_address,
                               uint32_t _quantity,
                               const uint16_t* _values)
{
    uint32_t counter = 0;

    if(!(_cli && _cli->req_adu && _cli->ans_adu && _quantity && _values))
        return -modbus_invalid_argument;

    if((uint32_t)_start_address + _quantity > 65536)
        return -modbus_invalid_argument;

    _cli->req_adu->server_id = _server_id;

    while(counter < _quantity) {
        uint32_t q = _quantity - counter;
        if(q > EMB_WRITE_REGS_MAX_QUANTITY)
            q = EMB_WRITE_REGS_MAX_QUANTITY;
        int res = __emb_sync_client_write_regs(_cli, _start_address + (uint16_t)counter, (uint16_t)q, _values + counter);
        if(res != modbus_success)
            return res;
        counter += q;
    }
    return modbus_success;
}

int emb_sync_client_rdwr_regs(emb_sync_client_t* _cli,
                              uint8_t _server_id,
                              uint16_t _wr_addr,
                              uint16_t _wr_quantity,
                              const uint16_t* _wr_values,
                              uint16_t _rd_addr,
                              uint16_t _rd_quantity,
                              uint16_t* _rd_values)
{
    int res;

    if(!(_cli && _cli->ans_adu && _cli->req_adu && _wr_values && _rd_values))
        return -modbus_invalid_argument;

    _cli->req_adu->server_id = _server_id;

    res = emb_rdwr_regs_make_req(&_cli->req_adu->pdu,
                                 _wr_addr,
                                 _wr_quantity,
                                 _wr_values,
                                 _rd_addr,
                                 _rd_quantity);
    if(res != modbus_success)
        return res;

    res = emb_sync_client_do_request(_cli, _cli->req_adu, _cli->ans_adu);
    if(res != modbus_success)
        return res;

    res = emb_rdwr_regs_get_answ_regs_n(MB_CONST_PDU(&_cli->ans_adu->pdu));
    if(res < 0)
        return res;
    else if(res != _rd_quantity)
        return -modbus_wrong_resp_quantity;

    return emb_rdwr_regs_get_answ_regs(MB_CONST_PDU(&_cli->ans_adu->pdu),
                                       0, (uint16_t)res, _rd_values);
}
