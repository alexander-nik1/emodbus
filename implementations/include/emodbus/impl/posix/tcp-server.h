
#ifndef EMB_TCP_SERVER_H
#define EMB_TCP_SERVER_H

#include <sys/types.h>
#include <netinet/in.h>

/*!
 * \file
 * \brief Definition of TCP server
 *
 * A TCP server.
 *
 * It has next features:
 * Connections limit
 *
 */

/*! \brief TCP Server context
 *
 */
typedef struct
{
    int rx_timeout_ms;                  ///< Receive timeout, must be set by user
    unsigned int max_connections;       ///< Connections limit, must be set by user

    // Next fileds is internal variables
    fd_set master;
    fd_set read_fds;
    struct sockaddr_in serveraddr;
    int fdmax;
    int listener;
	unsigned int conn_counter;
} emb_tcp_server_t;

/**
 * @brief Initialize
 *
 * Use this funtion to initialize context
 * This funtion should be called before using
 * a TCP server.
 *
 * @param[in] _srv    TCP server context
 * @param[in] _addr   Address to bind
 * @param[in] _port   Port to bind
 * @return Zero if ok, or negative error code, if errors.
 */
int emb_tcp_server_init(emb_tcp_server_t* _srv, in_addr_t _addr, uint16_t _port);

/**
 * @brief Deinitialize
 *
 * Call this function on your context to
 * free all resources.
 *
 * @param[in] _srv  TCP server context
 * @return Zero if ok, or negative error code, if errors.
 */
int emb_tcp_server_deinit(emb_tcp_server_t* _srv);

/**
 * @brief Receive
 *
 * Call this function to receive data from one of clients.
 * This function also controls connections.
 * Function may return 0, meaning accepting a new connection or closing it.
 *
 * @param[in] _srv          TCP server context
 * @param[out] _client_id   A place to store client's id. (this id must be used in a send function \see emb_tcp_server_send)
 * @param[out] _buffer      A place to store data in it.
 * @param[in] _buf_size     A size of the data place.
 * @return Positive value, if some data was received. Zero, if there was a connenction or disconnection. Negative, if errors occured.
 */
int emb_tcp_server_receive(emb_tcp_server_t* _srv, int* _client_id,
                           uint8_t* _buffer, unsigned int _buf_size);

/**
 * @brief Send
 *
 * Call this function to send data to one of clients.
 *
 * @param[in] _srv          TCP server context
 * @param[in] _client_id    The client's ID to determine, which client must receive this data
 * @param[in] _data         The data to send.
 * @param[in] _data_length  A sizef of the data to send.
 * @return Positive value, if some data was sent. Negative, if errors occured.
 */
int emb_tcp_server_send(emb_tcp_server_t* _srv, int _client_id,
                        const uint8_t* _data, unsigned int _data_length);

#endif // EMB_TCP_SERVER_H
