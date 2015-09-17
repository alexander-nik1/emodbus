
#include "rtu.h"
#include "add/container_of.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "byte-word.h"
#include "add/crc.h"

static void parse_packet(struct modbus_rtu_t* _mbt) {
    const unsigned char* buf = _mbt->rx_buffer;
    const unsigned int size = _mbt->rx_buf_counter;
    if(size >= 4) { // 4 bytes - minimal packet size
        const uint16_t crc1 = crc16(buf, size-2);
        const uint16_t crc2 = MKWORD(buf[size-2], buf[size-1]);
        if(crc1 == crc2) {
            _mbt->modbus_rtu_on_packet(_mbt->user_data, buf, size);
        }
    }
}

static int modbus_rtu_on_write(struct input_stream_t* _this, const void* _data, unsigned int _size) {

    const unsigned char* data = (unsigned char*)_data;
    struct modbus_rtu_t* mbt = container_of(_this, struct modbus_rtu_t, input_stream);

    if((mbt->rx_buf_counter + _size) <= mbt->rx_buf_size) {
        memcpy(mbt->rx_buffer + mbt->rx_buf_counter, _data, _size);
        mbt->rx_buf_counter += _size;
    }

    mbt->modbus_rtu_on_char(mbt->user_data);

    return _size;
}

static int modbus_rtu_on_read(struct output_stream_t* _this, void* _data, unsigned int _size) {
    struct modbus_rtu_t* mbt = container_of(_this, struct modbus_rtu_t, output_stream);
    if(mbt->tx_buf_counter < mbt->tx_pkt_size) {
        const unsigned int remainder = mbt->tx_pkt_size - mbt->tx_buf_counter;
        const unsigned int sz = _size > remainder ? remainder : _size;
        memcpy(_data, mbt->tx_buffer + mbt->tx_buf_counter, sz);
        mbt->tx_buf_counter += sz;
        return sz;
    }
    return 0;
}

void modbus_rtu_initialize(struct modbus_rtu_t* _mbt) {
    _mbt->input_stream.on_write = modbus_rtu_on_write;
    _mbt->output_stream.on_read = modbus_rtu_on_read;
    _mbt->rx_buf_counter = 0;
    _mbt->tx_buf_counter = 0;
    _mbt->tx_pkt_size = 0;
}

void modbus_rtu_on_symbol_timeout(struct modbus_rtu_t* _mbt) {
    parse_packet(_mbt);
    _mbt->rx_buf_counter = 0;
}

int modbus_rtu_send_packet(struct modbus_rtu_t* _mbt, const void* _pkt, unsigned int _size) {
    if((_size + 2) <= _mbt->tx_buf_size) {
        const uint16_t crc = crc16(_pkt, _size);
        memcpy(_mbt->tx_buffer, _pkt, _size);
        memcpy(_mbt->tx_buffer + _size, &crc, 2);
        _mbt->tx_pkt_size = _size + 2;
        // write first part
        _mbt->tx_buf_counter = stream_write(&_mbt->output_stream, _mbt->tx_buffer, _mbt->tx_pkt_size);
        return 0;
    }
    else
        return -1;
}

int modbus_rtu_send_packet_sync(struct modbus_rtu_t* _mbt, const void* _pkt, unsigned int _size) {
    int r;
    if((r = modbus_rtu_send_packet(_mbt, _pkt, _size)) == 0) {
        while(_mbt->tx_buf_counter < _mbt->tx_pkt_size) {
            const unsigned int remainder = _mbt->tx_pkt_size - _mbt->tx_buf_counter;
            _mbt->tx_buf_counter += stream_write(&_mbt->output_stream, _mbt->tx_buffer, remainder);
        }
        return 0;
    }
    else
        return r;
}
