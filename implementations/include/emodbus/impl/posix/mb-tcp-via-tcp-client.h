
#ifndef EMB_TCP_VIA_TCP_CLIENT_H
#define EMB_TCP_VIA_TCP_CLIENT_H

#include <emodbus/base/modbus_transport.h>
#include <emodbus/impl/posix/tcp-client.h>
#include <event2/event.h>

#ifdef __cplusplus
extern "C" {
#endif

struct emb_tcp_via_tcp_client_t;

typedef void (*emb_tcp_via_tcp_client_notifier_t)(void* _param,
                                                  enum tcp_client_events_t _event);

struct emb_tcp_via_tcp_client_t*
emb_tcp_via_tcp_client_create(struct event_base *_base,
                           const char* _ip_addr,
                           unsigned int _port);

void emb_tcp_via_tcp_client_destroy(struct emb_tcp_via_tcp_client_t* _ctx);

struct emb_transport_t*
emb_tcp_via_tcp_client_get_transport(struct emb_tcp_via_tcp_client_t* _ctx);

int emb_tcp_via_tcp_client_set_notifier(struct emb_tcp_via_tcp_client_t* _ctx,
                                        emb_tcp_via_tcp_client_notifier_t _notifier,
                                        void* _param);

#ifdef __cplusplus
};
#endif

#endif // EMB_TCP_VIA_TCP_CLIENT_H
