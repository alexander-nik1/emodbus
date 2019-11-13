
#ifndef EMODBUS_SERVER_BASE_H
#define EMODBUS_SERVER_BASE_H

#include <emodbus/base/modbus_adu.h>
#include <emodbus/server/bits.h>
#include <emodbus/server/regs.h>
#include <emodbus/server/file.h>
#include <stdint.h>

/*!
 * \file
 * \brief Server.
 *
 * This file contains a Server part of modbus.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Events of Server
 *
 * This enum used as parameter in a
 * emb_super_server_t::on_event function
 *
 */
enum emb_super_server_event_t
{
    embsev_on_receive_pkt,      ///< Packet was received
    embsev_no_srv,              ///< Server was not found for request's id
    embsev_mb_exception,        ///< Some exception was sent as response to incorrect request
    embsev_resp_sent,           ///< Response was sent
    embsev_transport_error      ///< There was an error in a transport level
};

struct emb_server_t;
struct emb_super_server_t;


/**
 * @brief Modbus function type
 *
 * This is definition of a modbus-function
 * This type used in a emb_server_t::get_function returning value
 * You will interest this only if you want to write your own implementation of
 * standart modbus functions, or write a new.
 *
 * @param [in] _ssrv Pointer to super-server object
 * @param [in] _srv Pointer to server object
 *
 * @return If 0, then answer must be in a emb_super_server_t::tx_pdu,
 * otherwise, an modbus-exception packet will be sent, error-code will be taken
 * from this returning value.
 *
 */
typedef uint8_t (*emb_srv_function_t)(struct emb_super_server_t* _ssrv, struct emb_server_t* _srv);

enum {
    EMB_SRV_BROADCAST_FLAG = (1 << 0)
};

/**
 * @brief Structure for represent a one server
 *
 * This structure describes a one server(slave) in Modbus terminology.
 *
 */
struct emb_server_t
{
    /**
     * @brief Get modbus function by number
     *
     * This is a user-defined function. By calling this function,
     *  EModbus takes a modbus-function for process each request.
     * You can return either function pointer from emodbus/emodbus/server/ files,
     * or write own custom function.
     *
     * @param [in] _srv Pointer to server object
     * @param [in] _func Number of function from request
     *
     * @return Pointer to function, or NULL if there are no such function
     *
     */
    emb_srv_function_t (*get_function)(struct emb_server_t* _srv, uint8_t _func);

    /**
     * @brief Get coils by address
     *
     * This is a user-defined function. By callin this function,
     *  EModbus takes a coils-range.
     *
     * @param [in] _srv Pointer to server object
     * @param [in] _begin Beginning address from request
     *
     * @return Pointer to a coils-range, or NULL if there are no such coils.
     *
     */
    struct emb_srv_bits_t* (*get_coils)(struct emb_server_t* _srv, uint16_t _begin);

    /**
     * @brief Get discrete inputs by address
     *
     * This is a user-defined function. By callin this function,
     *  EModbus takes a discrete-inputs-range.
     *
     * @param [in] _srv Pointer to server object
     * @param [in] _begin Beginning address from request
     *
     * @return Pointer to a discrete-inputs-range, or NULL if there are no such discrete-inputs.
     *
     */
    struct emb_srv_bits_t* (*get_discrete_inputs)(struct emb_server_t* _srv, uint16_t _begin);

    /**
     * @brief Get holdings by address
     *
     * This is a user-defined function. By callin this function,
     *  EModbus takes a holdings-range.
     *
     * @param [in] _srv Pointer to server object
     * @param [in] _begin Beginning address from request
     *
     * @return Pointer to a holdings-range, or NULL if there are no such holdings.
     *
     */
    struct emb_srv_regs_t* (*get_holding_regs)(struct emb_server_t* _srv, uint16_t _begin);

    /**
     * @brief Get input-regs by address
     *
     * This is a user-defined function. By callin this function,
     *  EModbus takes a input-regs.
     *
     * @param [in] _srv Pointer to server object
     * @param [in] _begin Beginning address from request
     *
     * @return Pointer to a input-regs-range, or NULL if there are no such input-regs.
     *
     */
    struct emb_srv_regs_t* (*get_input_regs)(struct emb_server_t* _srv, uint16_t _begin);

    /**
     * @brief Get modbus-file by file-number
     *
     * This is a user-defined function. By callin this function,
     *  EModbus takes a modbus-file.
     *
     * @param [in] _srv Pointer to server object
     * @param [in] _fileno File number from request
     *
     * @return Pointer to a modbus-file, or NULL if there are no such modbus-file.
     *
     */
    struct emb_srv_file_t* (*get_file)(struct emb_server_t* _srv, uint16_t _fileno/*, uint16_t _begin*/);

    /**
     * @brief Read FIFO implementation
     *
     * This is a user-defined function. By callin this function,
     *  EModbus asks to process a Read-FIFO function. User must write into
     * this fifo all data, but not more than EMB_SRV_READ_FIFO_MAX_REGS registers.
     *
     * @param [in] _srv Pointer to server object
     * @param [in] _address FIFO address
     * @param [out] _fifo_buf A place to write data into
     * @param [out] _fifo_count Number of written registers
     *
     * @return Pointer to a modbus-file, or NULL if there are no such modbus-file.
     *
     */
    uint8_t (*read_fifo)(struct emb_server_t* _srv, uint16_t _address,
                         uint16_t* _fifo_buf, uint8_t* _fifo_count);

    /**
     * @brief Flags for server
     *
     * EMB_SRV_BROADCAST_FLAG - Do not send answer anyway.
     *
     */
    uint8_t flags;
};

/**
 * @brief Structure for represent a super-server
 *
 * This is a structure to manage a modbus-servers.
 * Only one object of this structure should be binded
 * to one physical port.
 *
 */
struct emb_super_server_t
{
    /**
     * @brief Get server by Id
     *
     * By calling this function, EModbus knows,
     * whether we have a server according given address.
     *
     * @param [in] _ssrv Pointer to super-server object
     * @param [in] _address Address(Id) of the server
     *
     * @return Pointer to server, or NULL if there are no such server.
     *
     */
    struct emb_server_t* (*get_server)(struct emb_super_server_t* _ssrv,
                                       uint8_t _address);

    /**
     * @brief A notifier of a server events.
     *
     * If you want to be notification about super-server's life,
     * you can set this pointer to you function.
     *
     * @param [in] _ssrv Pointer to super-server object
     * @param [in] _event Look at @emb_super_server_event_t
     * @param [in] _data Some data, depending of event type
     *
     */
    void (*on_event)(struct emb_super_server_t* _ssrv, enum emb_super_server_event_t _event, uint8_t _data);


    emb_pdu_t* rx_pdu;  ///< Current received PDU
    emb_pdu_t* tx_pdu;  ///< Current PDU for sending
};

/**
 * @brief Initialize the super-server
 *
 * Before any use of super-server object,
 * you need to initialize it by calling this function
 *
 * @param [in] _ssrv Pointer to super-server object
 *
 */
void emb_super_server_init(struct emb_super_server_t* _ssrv);


int emb_build_exception_pdu(emb_pdu_t* _result,
                            uint8_t _func,
                            uint8_t _errno);

/**
 * @brief Process request
 *
 * User must call this function when he wants to
 * process request by this super-server.
 *
 * @param [in] _ssrv Pointer to super-server object
 * @param [in] _rx_adu Pointer to received resuest
 * @param [out] _tx_adu Pointer to request for send as answer
 *
 * @return if 1, then you have an answer(it written to @_tx_adu) to send,
 * if 0, then nothing to send as answer.
 */
int emb_super_server_process_req(struct emb_super_server_t* _ssrv,
                                 const  emb_adu_t* _rx_adu,
                                 emb_adu_t* _tx_adu);

//**********************************************************************
// Coils and Discrete inputs.

uint8_t emb_srv_read_bits(struct emb_super_server_t* _ssrv,
                          struct emb_server_t* _srv);

uint8_t emb_srv_write_coil(struct emb_super_server_t* _ssrv,
                           struct emb_server_t* _srv);

uint8_t emb_srv_write_coils(struct emb_super_server_t* _ssrv,
                            struct emb_server_t* _srv);

//**********************************************************************
// Holding and Input registers

uint8_t emb_srv_read_regs(struct emb_super_server_t* _ssrv,
                          struct emb_server_t* _srv);

uint8_t emb_srv_write_reg(struct emb_super_server_t* _ssrv,
                          struct emb_server_t* _srv);

uint8_t emb_srv_write_regs(struct emb_super_server_t* _ssrv,
                           struct emb_server_t* _srv);

uint8_t emb_srv_mask_reg(struct emb_super_server_t* _ssrv,
                         struct emb_server_t* _srv);


enum { EMB_SRV_RDWR_REGS_MAX_READ_REGS = 0x007D };
enum { EMB_SRV_RDWR_REGS_MAX_WRITE_REGS = 0x0079 };

uint8_t emb_srv_read_write_regs(struct emb_super_server_t* _ssrv,
                                struct emb_server_t* _srv);

//**********************************************************************
// File records

uint8_t emb_srv_read_file(struct emb_super_server_t* _ssrv,
                          struct emb_server_t* _srv);

uint8_t emb_srv_write_file(struct emb_super_server_t* _ssrv,
                           struct emb_server_t* _srv);

//**********************************************************************
// FIFOs

enum { EMB_SRV_READ_FIFO_MAX_REGS = 31 };

uint8_t emb_srv_read_fifo(struct emb_super_server_t* _ssrv,
                          struct emb_server_t* _srv);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // EMODBUS_SERVER_BASE_H

