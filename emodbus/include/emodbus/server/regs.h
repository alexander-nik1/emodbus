
#ifndef EMODBUS_SERVER_BITS_H
#define EMODBUS_SERVER_BITS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Server, registers definitions
 *
 * It used in the 0x03,0x04,0x06,0x10,0x16,0x17 modbus-functions.
 *
 */

/**
 * @brief Registers range
 *
 * A range to represent a holding-registers and
 * input-registers.
 */

struct emb_srv_regs_t {

    uint16_t start; ///< Begining address of registers range
    uint32_t size;  ///< Nuber of registers in the range

    /**
     * @brief Read registers
     *
     * This is a user-defined function.
     * Function called by a modbus-functions to
     * request the registers from user.
     *
     * @param [in] _rr The regs-range (holdings or input-registers)
     * @param [in] _offset Offset inside this range, to begin reading from
     * @param [in] _quantity Number of registers to read
     * @param [out] _pvalues A place for write registers into
     *
     * @return If 0, then all is ok,
     * otherwise, an modbus-exception packet will be sent, error-code will be taken
     * from this returning value.
     *
     * @see emb_srv_read_regs
     * @see emb_srv_mask_reg
     * @see emb_srv_read_write_regs
     */
    uint8_t (*read_regs)(struct emb_srv_regs_t* _rr,
                         uint16_t _offset,
                         uint16_t _quantity,
                         uint16_t* _pvalues);

    /**
     * @brief Write registers
     *
     * This is a user-defined function.
     * Function called by a modbus-functions to
     * send registers to user.
     *
     * @param [in] _coils The regs-range (holdings or input-registers)
     * @param [in] _offset Offset inside this range, to begin write from
     * @param [in] _quantity Number of registers to write
     * @param [in] _pvalues A place for take registers from
     *
     * @return If 0, then all is ok,
     * otherwise, an modbus-exception packet will be sent, error-code will be taken
     * from this returning value.
     *
     * @see emb_srv_write_reg
     * @see emb_srv_write_regs
     * @see emb_srv_mask_reg
     * @see emb_srv_read_write_regs
     */
    uint8_t (*write_regs)(struct emb_srv_regs_t* _rr,
                          uint16_t _offset,
                          uint16_t _quantity,
                          const uint16_t* _pvalues);
};

#ifdef __cplusplus
}   // extern "C"
#endif

#endif //EMODBUS_SERVER_BITS_H
