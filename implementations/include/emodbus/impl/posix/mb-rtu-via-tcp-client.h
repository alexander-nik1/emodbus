#ifndef EMB_RTU_VIA_TCP_CLIENT_H
#define EMB_RTU_VIA_TCP_CLIENT_H

#include <emodbus/base/modbus_transport.h>
#include <event2/event.h>

#ifdef __cplusplus
extern "C" {
#endif

/// The RTU via TCP-Server context
struct emb_rtu_via_tcp_client_t;

/**
 * @brief emb_rtu_via_tcp_client_create
 *
 * Allocates the memory for context, and creates a
 * TCP client with given options.
 * If this finction had returned non-zero value, then
 * you need to free this context by emb_rtu_via_tcp_client_destroy call.
 *
 * @param[in] _base Libevent base.
 * @param[in] _timeout_ms The pause between symbols, by which determines an end of the packet.
 * @param[in] _ip_addr IP address to connect
 * @param[in] _port Port to connect
 * @return The RTU via TCP-Server context, NULL if there are errors.
 *
 * @see emb_rtu_via_tcp_client_destroy
 */
struct emb_rtu_via_tcp_client_t*
emb_rtu_via_tcp_client_create(struct event_base *_base,
                              unsigned int _timeout_ms,
                              const char* _ip_addr,
                              unsigned int _port);

/**
 * @brief emb_rtu_via_tcp_client_set_timeout
 *
 * Set the pause between symbols, by which determines an end of the packet.
 *
 * @param[in] _ctx The RTU via TCP-Server context
 * @param[in] _timeout_ms The pause, value in milliseconds.
 */
void emb_rtu_via_tcp_client_set_timeout(struct emb_rtu_via_tcp_client_t* _ctx,
                                        unsigned int _timeout_ms);

/**
 * @brief emb_rtu_via_tcp_client_destroy
 *
 * Close the RTU via TCP-Server context.
 * And free all previously allocated memory.
 *
 * @param[in] _ctx The RTU via TCP-Server context
 *
 * @see emb_rtu_via_tcp_client_create
 */
void emb_rtu_via_tcp_client_destroy(struct emb_rtu_via_tcp_client_t* _ctx);

/**
 * @brief emb_rtu_via_tcp_client_get_transport
 *
 * Get the transport for this context.
 *
 * @param[in] _ctx The RTU via TCP-Server context
 * @return The emodbus transport, to use it in a high level.
 */
struct emb_transport_t*
emb_rtu_via_tcp_client_get_transport(struct emb_rtu_via_tcp_client_t* _ctx);

/**
 * @brief emb_rtu_via_tcp_cli_get_tcp_client
 *
 * Get the pointer to tcp_client_t.
 *
 * @param _ctx The RTU via TCP-Server context
 * @return The pointer to the tcp_client_t.
 */
struct tcp_client_t*
emb_rtu_via_tcp_cli_get_tcp_client(struct emb_rtu_via_tcp_client_t* _ctx);

#ifdef __cplusplus
}
#endif

#endif // EMB_RTU_VIA_TCP_CLIENT_H

