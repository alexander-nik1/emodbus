
#ifndef EMODBUS_SERVER_COILS_H
#define EMODBUS_SERVER_COILS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Server, bits definitions
 *
 * It used in the 0x01,0x02,0x05,0x0F modbus-functions.
 *
 */

/**
 * @brief Bits range
 *
 * A range to represent a modbus-coils and
 * modbus-discrete-inputs.
 *
 */
struct emb_srv_bits_t
{
    uint16_t start; ///< Begining address of bits range
    uint32_t size;  ///< Nuber of bits in the range

    /**
     * @brief Read bits
     *
     * This is a user-defined function.
     * Function called by a emb_srv_read_bits function to
     * request bits from user.
     *
     * @param [in] _coils The bits-range (coils or discrete-inputs)
     * @param [in] _offset Offset inside this range, to begin reading from
     * @param [in] _quantity Number of coils to read
     * @param [out] _pvalues A place for write bits into
     *
     * @return If 0, then all is ok,
     * otherwise, an modbus-exception packet will be sent, error-code will be taken
     * from this returning value.
     *
     * @see emb_srv_read_bits
     */
    uint8_t (*read_bits)(struct emb_srv_bits_t* _coils,
						 uint16_t _offset,
						 uint16_t _quantity,
						 uint8_t* _pvalues);

    /**
     * @brief Write bits
     *
     * This is a user-defined function.
     * Function called by a emb_srv_write_coil and emb_srv_write_coils functions to
     * send bits to user.
     *
     * @param [in] _coils The coils-range
     * @param [in] _offset Offset inside this range, to begin write from
     * @param [in] _quantity Number of coils to write
     * @param [in] _pvalues A place for take bits from
     *
     * @return If 0, then all is ok,
     * otherwise, an modbus-exception packet will be sent, error-code will be taken
     * from this returning value.
     *
     * @see emb_srv_write_coil
     * @see emb_srv_write_coils
     */
    uint8_t (*write_bits)(struct emb_srv_bits_t* _coils,
						  uint16_t _offset,
						  uint16_t _quantity,
						  const uint8_t* _pvalues);
};

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // EMODBUS_SERVER_COILS_H
