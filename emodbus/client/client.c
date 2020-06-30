
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
#include <string.h>
#include <errno.h>

/*!
 * \file
 * \brief Realisation of modbus client (master) side.
 *
 */

int emb_sync_client_do_request(emb_sync_client_t* _cli, const emb_adu_t* _req_adu, emb_adu_t* _ans_adu)
{
    int r;
    if(!(_cli && _cli->recv_adu && _cli->send_adu && _req_adu && _ans_adu)) {
        return -modbus_invalid_argument;
    }

    r = _cli->send_adu(_cli, _req_adu);
    if(r != modbus_success)
        return r;

    r = _cli->recv_adu(_cli, _ans_adu);
    if(r != modbus_success)
        return r;

    if(_req_adu->server_id != _ans_adu->server_id)
        return -modbus_resp_wrong_address;

    return emb_check_pdu_for_exception(&_ans_adu->pdu);
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

    if(!(_cli && _cli->req_adu && _cli->ans_adu && _quantity && _result)) {
        return -modbus_invalid_argument;
    }

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
