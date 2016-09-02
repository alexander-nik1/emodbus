
#ifndef EMB_TCP_SERVER_TCP
#define EMB_TCP_SERVER_TCP

#include <emodbus/base/modbus_proto.h>
#include <emodbus/base/modbus_pdu.h>
#include <event2/event.h>

#ifdef __cplusplus
extern "C" {
#endif

struct tcp_server_tcp_t;

struct tcp_server_tcp_t* tcp_server_tcp_new(struct event_base *_base,
                                            int _port);

void tcp_server_tcp_free(struct tcp_server_tcp_t* _ctx);

struct emb_protocol_t* tcp_server_tcp_get_proto(struct tcp_server_tcp_t* _ctx);

#ifdef __cplusplus
};
#endif

#endif // EMB_TCP_CLIENT_TCP
