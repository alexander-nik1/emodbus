
#ifndef EMB_TCP_CLIENT_H
#define EMB_TCP_CLIENT_H

#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

/*!
 * \file
 * \brief Definition of TCP client
 *
 * A TCP client for building a stable
 * data exchange channel on it.
 *
 * It has next features:
 * Automatic reconnect (when errors occured),
 * forced reconnect (by specifying certain time in seconds)
 * receive timeout (milliseconds),
 * connection timeout (milliseconds),
 * delay between reconnection attempts,
 * separated delay for first try and separated delay for all next attempts.
 * Forced reconnect can be executed at sending or,and receiving (selects by flags)
 * Also, to provide constant sending and receiving time, you can set flag, to
 * execute a staged connection. It will not add
 * additional delays to await connection.
 *
 */

/*!
 * \brief States of TCP Client
 */
typedef enum
{
	emb_tcs_connecting,
	emb_tcs_connected,
	emb_tcs_disconnected
} emb_tcp_client_state_t;

/*!
 * \brief TCP client flags
 */
typedef enum
{
	EMB_TCP_CLI_NO_DELAY_WHILE_CONNECT = (1 << 0),	///< Set this flag will not add additional delays to await connection.
	EMB_TCP_CLI_FORCE_RECONN_AT_SEND =   (1 << 1),	///< Allow forced reconnection after successful send operation
	EMB_TCP_CLI_FORCE_RECONN_AT_RECV =   (1 << 2),	///< Allow forced reconnection after successful receive operation
	EMB_TCP_CLI_RECONNECT_AT_TIMEOUTS_COUNTER = (1 << 3) ///< Reconnect when a timeouts counter is reached defined value
} emb_tcp_cli_flags_t;

/*! \brief TCP Client context
 *
 */
typedef struct
{
	unsigned int transmit_timeout_ms;			///< Transmit timeout, must be set by user
	unsigned int receive_timeout_ms;			///< Receive timeout, must be set by user
	unsigned int connect_timeout_ms;			///< Connection timeout, must be set by user
	unsigned int first_reconnect_delay_ms;		///< Delay between disconnect and connection attempt, must be set by user
	unsigned int next_reconnects_delay_ms;		///< Delay between connection attempts, must be set by user
	unsigned int force_reconnect_delay_s;		///< The time after which there will be a forced reconnection.
												///< The countdown starts from the moment of successful connection. Must be set by user
	unsigned int rxtx_timeouts_to_reconnect;	///< The maximum of timeout errors, by reaching which, a reconnect will be triggered.

	struct sockaddr_in serveraddr;				///< Internal variable (used for passing connection options)
	int fd;										///< Internal variable (socket descriptor)
	unsigned int flags;							///< Flags \see emb_tcp_cli_flags_t, must be set by user
	char is_first_reconnect;					///< Internal variable (a flag for first try of connect)

	emb_tcp_client_state_t state;				///< Internal variable (state of client, \see emb_tcp_client_state_t)
	struct timeval connection_start_time;		///< Internal variable (time, when connection started)
	struct timeval connect_time;				///< Internal variable (time, when connection estabilished)
	struct timeval disconnect_time;				///< Internal variable (disconnection time)

	unsigned long long rx_bytes;				///< Statistic, received bytes
	unsigned long long tx_bytes;				///< Statistic, transmitted bytes
	unsigned int connection_attempts;			///< Statistic, number of connection attempts
	unsigned int rx_timeouts_counter;			///< Counter of timeouts for receiving, increases when timeout occurs, decreases, when is ok
	unsigned int tx_timeouts_counter;			///< Counter of timeouts for transmitting, increases when timeout occurs, decreases, when is ok
} emb_tcp_client_t;

/**
 * @brief Set connection options
 *
 * Use this function to set IP address and port,
 * that will be used for connection.
 *
 * @param[in,out] _cli TCP client context
 * @param[in] _ip Server's IP Address like: "192.168.1.66"
 * @param[in] _port Server's Port
 * @return Zero if ok, or negative error code, if errors.
 */
int emb_tcp_client_set_connection_options(emb_tcp_client_t* _cli, const char* _ip, uint16_t _port);

/**
 * @brief Initialize
 *
 * This function is used for initialize context,
 * it must be called before any other functions.
 *
 * @param[in,out] _cli TCP client context
 * @return Zero if ok, or negative error code, if errors.
 */
int emb_tcp_client_init(emb_tcp_client_t* _cli);

/**
 * @brief Deinitailize
 *
 * This function is used to disable context and close connection.
 *
 * @param[in,out] _cli TCP client context
 * @return Zero if ok, or negative error code, if errors.
 */
int emb_tcp_client_deinit(emb_tcp_client_t* _cli);

/**
 * @brief Send data
 *
 * This function is used to send data.
 *
 * @param[in,out] _cli TCP client context
 * @param[in] _buf Pointer to data
 * @param[in] _length Number of bytes to send
 * @return Number of sent bytes, or negative error code, if errors.
 */
int emb_tcp_client_send(emb_tcp_client_t* _cli, const void* _buf, unsigned int _length);

/**
 * @brief Receive data
 *
 * This function is used to receive a ModbusTCP frame
 *
 * @param[in,out] _cli TCP client context
 * @param[out] _buf Pointer to place for received data
 * @param[in] _length Number of bytes to receive
 * @return Number of received bytes, or negative error code, if errors.
 */
int emb_tcp_client_recv_tcp(emb_tcp_client_t* _cli, void* _buf, unsigned int _length);

#endif // EMB_TCP_CLIENT_H
