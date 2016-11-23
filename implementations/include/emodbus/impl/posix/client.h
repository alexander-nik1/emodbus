
#ifndef EMB_POSIX_CLIENT_IMPL_H
#define EMB_POSIX_CLIENT_IMPL_H

#include <emodbus/client/client.h>
#include <emodbus/base/modbus_transport.h>

#include <event2/event.h>

#ifdef __cplusplus
extern "C" {
#endif

struct emb_posix_sync_client_t;


struct emb_posix_sync_client_t*
emb_posix_sync_client_create(struct event_base *_base);

void emb_posix_sync_client_set_transport(struct emb_posix_sync_client_t* _cli,
                                         struct emb_transport_t* _transport);

void emb_posix_sync_client_destroy(struct emb_posix_sync_client_t* _cli);

int emb_posix_sync_client_transaction(struct emb_posix_sync_client_t* _cli,
                                      int _server_addr,
                                      unsigned int _timeout,
                                      struct emb_client_transaction_t* _transact);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // EMB_POSIX_CLIENT_IMPL_H
