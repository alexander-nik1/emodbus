
#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <event2/event.h>

#ifdef __cplusplus
extern "C" {
#endif

// A default pause between tries of reconnection.
// A value in seconds.
enum { TCP_CLIENT_DEFAULT_RECONNECT_DLY = 10 };

enum tcp_client_events_t {
    tcp_cli_data_received,
    tcp_cli_data_sent,
    tcp_cli_connected,
    tcp_cli_disconnected,
    tcp_cli_bad_try_of_reconnection
};

enum tcp_client_state_t {
    tcp_client_default,
    tcp_client_disconnected,
    tcp_client_connected
};

struct tcp_client_t;

typedef void (*tcp_cient_notifier_t)(struct tcp_client_t* _ctx,
                                     enum tcp_client_events_t _event);

struct tcp_client_t* tcp_client_new(struct event_base* _base,
                                    tcp_cient_notifier_t _event_notifier);

void tcp_client_free(struct tcp_client_t* _ctx);

int tcp_client_start_connection(struct tcp_client_t* _ctx,
                                const char* _ip_address,
                                unsigned short _port);

void tcp_client_stop_connection(struct tcp_client_t* _ctx);

int tcp_client_read(struct tcp_client_t* _ctx,
                    void* _p_buffer,
                    size_t _buff_size);

int tcp_client_write(struct tcp_client_t* _ctx,
                     const void* _p_data,
                     size_t _data_size);

void tcp_client_enable_write_event(struct tcp_client_t* _ctx);

struct event_base* tcp_client_get_base(const struct tcp_client_t* _ctx);

int tcp_client_get_fd(const struct tcp_client_t* _ctx);

void tcp_client_set_reconnection_delay(struct tcp_client_t* _ctx,
                                       unsigned int _sec);

void tcp_client_set_user_data(struct tcp_client_t* _ctx, void* _user_data);

void* tcp_client_get_user_data(struct tcp_client_t* _ctx);

#ifdef __cplusplus
};
#endif

#endif // TCP_CLIENT_H
