
#include <emodbus/client/client.h>
#include <emodbus/base/modbus_errno.h>
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

    return emb_check_pdu_for_exception(&_ans_adu->pdu);
}
