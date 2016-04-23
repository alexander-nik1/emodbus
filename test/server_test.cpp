
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <list>
#include <errno.h>
#include <string.h>
#include <bitset>

#include <emodbus/emodbus.hpp>

#include <emodbus/base/common.h>
#include <emodbus/base/modbus_errno.h>

#include <emodbus/server/server.h>
#include <emodbus/server/coils.h>
#include <emodbus/server/holdings.h>
#include <emodbus/server/file.h>

#include <emodbus/base/add/stream.h>

#include <event2/event.h>

#include "timespec_operations.h"

#include "posix_serial_rtu/posix_serial_rtu.hpp"
#include "dumping_helper.hpp"

class my_coils_t : public emb::server::coils_t {
public:
    enum { START = 0x0000 };
    enum { SIZE = 0xFFFF };

    my_coils_t() : emb::server::coils_t(START, SIZE) {
        values.resize(SIZE);
        for(int i=0; i<SIZE; ++i) {
            values[i] = false;
        }
    }

private:
    uint8_t on_read_coils(uint16_t _offset,
                          uint16_t _quantity,
                          uint8_t* _pvalues) {
//        printf("read_coils offset:0x%04X,quantity:0x%04X values:", _offset, _quantity);
        for(int byte=0; _quantity != 0; ++byte) {
            _pvalues[byte] = 0;
            const int n_bits = _quantity > 8 ? 8 : _quantity;
            for(int bit=0; bit<n_bits; ++bit) {
                if(values[_offset + byte*8 + bit]) {
                    _pvalues[byte] |= (1 << bit);
                }
//                printf("%d ", (int)values[_offset + byte*8 + bit]);
            }
            _quantity -= n_bits;
        }
//        printf("\n");
//        fflush(stdout);
        return 0;
    }

    uint8_t on_write_coils(uint16_t _offset,
                           uint16_t _quantity,
                           const uint8_t* _pvalues) {

//        printf("write_coils offset:0x%04X,quantity:0x%04X values:", _offset, _quantity);

        for(int byte=0; _quantity != 0; ++byte) {
            const int n_bits = _quantity > 8 ? 8 : _quantity;
            for(int bit=0; bit<n_bits; ++bit) {
                values[_offset + byte*8 + bit] = (_pvalues[byte] & (1 << bit)) != 0;

//                printf("%d ", (int)values[_offset + byte*8 + bit]);
            }
            _quantity -= n_bits;
        }
//        printf("\n");
//        fflush(stdout);
        return 0;
    }

    std::vector<bool> values;
};

class my_holdings_t : public emb::server::holdings_t {
public:

    enum { START = 0x0000 };
    enum { SIZE = 0xFFFF };

    my_holdings_t() : emb::server::holdings_t(START, SIZE) {

        regs.resize(SIZE);
        memset(&regs[0], 0, SIZE*2);
    }

    uint8_t on_read_regs(uint16_t _offset,
                         uint16_t _quantity, uint16_t* _pvalues) {

        memcpy(_pvalues, &regs[_offset], _quantity*2);

        printf("Read Holdings: start:0x%04X, length:0x%04X\n", _offset, _quantity);
        fflush(stdout);
        return 0;
    }

    uint8_t on_write_regs(uint16_t _offset,
                          uint16_t _quantity, const uint16_t* _pvalues) {

        printf("Write Holdings: start:0x%04X, length:0x%04X\nData:", _offset, _quantity);
        for(int i=0; i<_quantity; ++i)
            printf("0x%04X ", _pvalues[i]);
        printf("\n");
        fflush(stdout);

        memcpy(&regs[_offset], _pvalues, _quantity*2);

        return 0;
    }
private:
    std::vector<uint16_t> regs;
};

class my_file_t : public emb::server::file_record_t {
public:
    enum { FILENO = 0 };
    enum { START = 0x0000 };
    enum { SIZE = 0xFFFF };

    my_file_t() : emb::server::file_record_t(FILENO/*, START, SIZE*/) {

        regs.resize(SIZE);
        memset(&regs[0], 0, SIZE*2);
    }

    uint8_t on_read_file(uint16_t _offset,
                         uint16_t _quantity, uint16_t* _pvalues) {

        memcpy(_pvalues, &regs[_offset], _quantity*2);

        printf("Read File: start:0x%04X, length:0x%04X\n", _offset, _quantity);
        fflush(stdout);
        return 0;
    }

    uint8_t on_write_file(uint16_t _offset,
                          uint16_t _quantity, const uint16_t* _pvalues) {

        printf("Write File: start:0x%04X, length:0x%04X\nData:", _offset, _quantity);
        for(int i=0; i<_quantity; ++i)
            printf("0x%04X ", _pvalues[i]);
        printf("\n");
        fflush(stdout);

        memcpy(&regs[_offset], _pvalues, _quantity*2);

        return 0;
    }
private:
    std::vector<uint16_t> regs;
};

emb_debug_helper_t emb_debug_helper;

int main(int argc, char* argv[]) {

    printf("emodbus server test\n");

    emb::server::super_server_t ssrv;

    struct event_base *base = event_base_new();

    posix_serial_rtu_t psp(base, "/dev/ttyUSB0", 115200);

    ssrv.set_proto(psp.get_proto());

    psp.get_proto()->flags |= EMB_PROTO_FLAG_DUMD_PAKETS;

    emb_debug_helper.enable_dumping();

    sleep(1);

    my_coils_t c;
    my_holdings_t h;
    my_file_t f;

    emb::server::server_t srv1(16);

    srv1.add_function(0x01, emb_srv_read_coils);
    srv1.add_function(0x05, emb_srv_write_coil);
    srv1.add_function(0x0F, emb_srv_write_coils);

    srv1.add_function(0x03, emb_srv_read_holdings);
    srv1.add_function(0x06, emb_srv_write_reg);
    srv1.add_function(0x10, emb_srv_write_regs);
    srv1.add_function(0x16, emb_srv_mask_reg);

    srv1.add_function(0x14, emb_srv_read_file);
    srv1.add_function(0x15, emb_srv_write_file);

    srv1.add_coils(c);
    srv1.add_holdings(h);
    srv1.add_file(f);

    ssrv.add_server(srv1);

    event_base_dispatch(base);

    return 0;
}
