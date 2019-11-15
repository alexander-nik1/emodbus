
#ifndef EMODBUS_SERVER_FILES_H
#define EMODBUS_SERVER_FILES_H

#include <stdint.h>
#include <emodbus/base/modbus_pdu.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Server, file-record definitions
 *
 * It used in the 0x14,0x15 modbus-functions.
 *
 */

/**
 * @brief file-record
 *
 * A structure to represent the file-record
 */

enum { EMB_FILE_REF_TYPE = 0x06 };

struct emb_srv_file_t {

	uint16_t fileno; ///< File number
//    uint16_t start;
//    uint16_t size;

	/**
	 * @brief Read file-record
	 *
	 * This is a user-defined function.
	 * Function called by a modbus-functions to
	 * request the registers from user.
	 *
	 * @param [in] _f The file-record object
	 * @param [in] _offset Offset inside this file-record, to begin reading from
	 * @param [in] _quantity Number of registers to read
	 * @param [out] _pvalues A place for write registers into
	 *
	 * @return If 0, then all is ok,
	 * otherwise, an modbus-exception packet will be sent, error-code will be taken
	 * from this returning value.
	 *
	 * @see emb_srv_read_file
	 */
    uint8_t (*read_file)(struct emb_srv_file_t* _f,
                         uint16_t _offset,
                         uint16_t _quantity,
                         uint16_t* _pvalues);

	/**
	 * @brief Write file-record
	 *
	 * This is a user-defined function.
	 * Function called by a modbus-functions to
	 * send registers to user.
	 *
	 * @param [in] _f The file-record object
	 * @param [in] _offset Offset inside this file-record, to begin write from
	 * @param [in] _quantity Number of registers to write
	 * @param [in] _pvalues A place for take registers from
	 *
	 * @return If 0, then all is ok,
	 * otherwise, an modbus-exception packet will be sent, error-code will be taken
	 * from this returning value.
	 *
	 * @see emb_srv_write_file
	 */
    uint8_t (*write_file)(struct emb_srv_file_t* _f,
                          uint16_t _offset,
                          uint16_t _quantity,
                          const uint16_t* _pvalues);
};

#ifdef __cplusplus
}   // extern "C"
#endif

#endif //EMODBUS_SERVER_FILES_H
