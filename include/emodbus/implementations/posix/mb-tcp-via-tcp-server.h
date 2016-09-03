
#ifndef EMB_TCP_VIA_TCP_SERVER_H
#define EMB_TCP_VIA_TCP_SERVER_H

#include <event2/event.h>

#ifdef __cplusplus
extern "C" {
#endif

struct emb_tcp_via_tcp_server_t;

struct emb_tcp_via_tcp_server_t*
emb_tcp_via_tcp_server_create(struct event_base *_base, int _port);

void emb_tcp_via_tcp_server_destroy(struct emb_tcp_via_tcp_server_t* _ctx);

struct emb_protocol_t*
emb_tcp_via_tcp_server_get_proto(struct emb_tcp_via_tcp_server_t* _ctx);

#ifdef __cplusplus
};
#endif

#endif // EMB_TCP_VIA_TCP_SERVER_H
