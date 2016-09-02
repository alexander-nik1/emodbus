
#ifndef EMB_TCP_CLIENT_TCP
#define EMB_TCP_CLIENT_TCP

#include <emodbus/protocols/tcp.h>
#include <emodbus/base/modbus_pdu.h>
#include <event2/event.h>

#ifdef __cplusplus
extern "C" {
#endif

struct emb_tcp_over_tcp_client_t;

struct emb_tcp_over_tcp_client_t*
emb_tcp_over_tcp_client_create(struct event_base *_base,
                               const char* _ip_addr,
                               unsigned int _port);

void emb_tcp_over_tcp_client_destroy(struct emb_tcp_over_tcp_client_t* _ctx);

struct emb_protocol_t*
emb_tcp_over_tcp_client_get_proto(struct emb_tcp_over_tcp_client_t* _ctx);

#ifdef __cplusplus
};
#endif

#endif // EMB_TCP_CLIENT_TCP
