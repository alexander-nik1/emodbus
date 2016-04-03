
#ifndef EMODBUS_SERVER_BASE_H
#define EMODBUS_SERVER_BASE_H

#include <emodbus/base/modbus_proto.h>
#include <emodbus/base/add/s-list.h>
#include <emodbus/server/holdings.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum emb_server_state_t {
    embs_default,
    embs_got_request,
    embs_processing_request,
    embs_sending_answer
};

struct emb_srv_function_t {

    uint8_t func_number;

    int (*function)(emb_const_pdu_t* _req, emb_pdu_t* _answer);

    struct sb_list_head list;
};

struct emb_server_t {

    /// Low level context
    struct emb_protocol_t* proto;

    /// The state of the modbus server
    enum emb_server_state_t state;

    /// Some data for high level
    void* user_data;

    struct sb_list_head funcs_anchor;

    struct sb_list_head list;
};

struct emb_super_server_t {

    struct sb_list_head anchor;
};

void emb_server_init(struct emb_server_t* _srv);



#ifdef __cplusplus
}   // extern "C"
#endif

#endif // EMODBUS_SERVER_BASE_H

