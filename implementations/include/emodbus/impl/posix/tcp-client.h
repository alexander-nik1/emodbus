
#ifndef EMB_TCP_CLIENT_H
#define EMB_TCP_CLIENT_H

#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

typedef enum
{
	emb_tcs_connecting,
	emb_tcs_connected,
	emb_tcs_disconnected
} emb_tcp_client_state_t;

#define EMB_TCP_CLI_NO_DELAY_CONNECT	(1 << 0)

typedef struct
{
    unsigned int receive_timeout_ms;
	unsigned int connect_timeout_ms;
    unsigned int first_reconnect_delay_ms;
    unsigned int next_reconnects_delay_ms;
    struct sockaddr_in serveraddr;
    int fd;
	unsigned int flags;
	char is_first_reconnect;

	emb_tcp_client_state_t state;
	struct timeval connection_start_time;

	unsigned long long rx_bytes;
	unsigned long long tx_bytes;
	unsigned int connection_attempts;

} emb_tcp_client_t;

int emb_tcp_client_set_connection_options(emb_tcp_client_t* _cli, const char* _ip, uint16_t _port);

int emb_tcp_client_init(emb_tcp_client_t* _cli);

int emb_tcp_client_deinit(emb_tcp_client_t* _cli);

int emb_tcp_client_send(emb_tcp_client_t* _cli, const void* _buf, unsigned int _length);

int emb_tcp_client_recv(emb_tcp_client_t* _cli, void* _buf, unsigned int _length);

#endif // EMB_TCP_CLIENT_H
