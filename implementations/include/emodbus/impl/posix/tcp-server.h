
#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <event2/event.h>

#ifdef __cplusplus
extern "C" {
#endif

enum tcp_server_events_t {
    tcp_srv_data_received,
    tcp_srv_data_sent
};

struct tcp_server_t;

typedef void (*tcp_server_notifier_t)(struct tcp_server_t* _ctx,
                                     void* _client_id,
                                     enum tcp_server_events_t _event);

struct tcp_server_t* tcp_server_new(struct event_base* _base,
                                    tcp_server_notifier_t _event_notifier,
                                    int _port);

void tcp_server_free(struct tcp_server_t* _ctx);

int tcp_server_read(struct tcp_server_t* _ctx,
                    void* _client_id,
                    void* _p_buffer,
                    size_t _buff_size);

int tcp_server_write(struct tcp_server_t* _ctx,
                     void* _client_id,
                     const void* _p_data,
                     size_t _data_size);

void tcp_server_set_user_data(struct tcp_server_t* _ctx, void* _user_data);

void* tcp_server_get_user_data(struct tcp_server_t* _ctx);

enum { tcp_server_unlimited_clients = -1 };

void tcp_server_set_clients_limit(struct tcp_server_t* _ctx,
                                  int _clients_limit);

int tcp_server_clients_limit(const struct tcp_server_t* _ctx);

#ifdef __cplusplus
};
#endif

#endif // TCP_SERVER_H
