
#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

typedef struct
{
    fd_set master;
    fd_set read_fds;
    struct sockaddr_in serveraddr;
    int fdmax;
    int listener;
} tcp_server_t;

int tcp_server_init(tcp_server_t* _srv, in_addr_t _addr, uint16_t _port);
int tcp_server_deinit(tcp_server_t* _srv);
int tcp_server_receive(tcp_server_t* _srv, int* _client_id,
                       uint8_t* _buffer, unsigned int _buf_size,
                       unsigned int _timeout_ms);
int tcp_server_send(tcp_server_t* _srv, int _client_id,
                    const uint8_t* _data, unsigned int _data_length);

#endif // TCP_SERVER_H
