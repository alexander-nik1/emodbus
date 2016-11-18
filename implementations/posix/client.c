
#include <emodbus/impl/posix/client.h>
#include <emodbus/base/modbus_errno.h>
#include <emodbus/base/add/container_of.h>

#include <memory.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

struct emb_posix_sync_client_t
{
    struct emb_client_t client;
    int transact_result;
    pthread_mutex_t mutex;
    struct event* timeout_timer;
};

// This function calls when response waiting timeout occurs.
static void timeout_cb(evutil_socket_t _fd, short _events, void * _arg) {
    struct emb_posix_sync_client_t* ctx = (struct emb_posix_sync_client_t*)_arg;
    emb_client_wait_timeout(&ctx->client);
    ctx->transact_result = -modbus_resp_timeout;
    pthread_mutex_unlock(&ctx->mutex);
}

// This function calls when a valid response was got from server
void client_on_response(struct emb_client_t* _req, int _slave_addr) {
    struct emb_posix_sync_client_t* ctx =
            container_of(_req, struct emb_posix_sync_client_t, client);
    ctx->transact_result=0;
    pthread_mutex_unlock(&ctx->mutex);
}

// This function calls when an error occurs, or when a modbus-error code was returned by a server.
void client_on_error(struct emb_client_t* _req, int _slave_addr, int _errno) {
    struct emb_posix_sync_client_t* ctx =
            container_of(_req, struct emb_posix_sync_client_t, client);
    ctx->transact_result = _errno;
    pthread_mutex_unlock(&ctx->mutex);
}

struct emb_posix_sync_client_t*
emb_posix_sync_client_create(struct event_base *_base)
{
    struct emb_posix_sync_client_t* res =
            (struct emb_posix_sync_client_t*)malloc(sizeof(struct emb_posix_sync_client_t));

    if(res) {
        memset(&res->client, 0, sizeof(struct emb_client_t));
        res->client.on_response = client_on_response;
        res->client.on_error = client_on_error;
        emb_client_init(&res->client);

        pthread_mutex_init(&res->mutex, NULL);
        pthread_mutex_trylock(&res->mutex);

        res->timeout_timer = event_new(_base, -1, EV_TIMEOUT, timeout_cb, res);

        res->transact_result = 0;
    }
    return res;
}

void emb_posix_sync_client_set_transport(struct emb_posix_sync_client_t* _cli,
                                         struct emb_transport_t* _transport)
{
    if(_cli)
        emb_client_set_transport(&_cli->client, _transport);
}

void emb_posix_sync_client_destroy(struct emb_posix_sync_client_t* _cli)
{
    if(_cli) {
        if(_cli->timeout_timer) {
            event_del(_cli->timeout_timer);
            event_free(_cli->timeout_timer);
        }
        free(_cli);
    }
}

int emb_posix_sync_client_transaction(struct emb_posix_sync_client_t* _cli,
                                      int _server_addr,
                                      unsigned int _timeout,
                                      struct emb_client_transaction_t* _transact)
{
    if(_cli) {
        int res;
        struct timeval timeout_time;

        timeout_time.tv_sec = 0;
        timeout_time.tv_usec = _timeout * 1000;

        if((res = emb_client_do_transaction(&_cli->client, _server_addr, _transact)) != 0)
            return res;

        event_add(_cli->timeout_timer, &timeout_time);

        pthread_mutex_lock(&_cli->mutex);
        event_del(_cli->timeout_timer);
        return _cli->transact_result;
    }

    return -EINVAL;
}
