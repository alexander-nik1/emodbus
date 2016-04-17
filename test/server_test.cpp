
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <list>
#include <errno.h>
#include <string.h>

#include <emodbus/emodbus.hpp>

#include <emodbus/base/common.h>
#include <emodbus/base/modbus_errno.h>

#include <emodbus/server/server.h>
#include <emodbus/server/holdings.h>
#include <emodbus/server/file.h>

#include <emodbus/base/add/stream.h>

#include <event2/event.h>

#include "timespec_operations.h"

#include "posix_serial_rtu/posix_serial_rtu.hpp"
#include "dumping_helper.hpp"

class my_holdings_t : public emb::server_holdings_t {
public:

    enum { START = 0x0000 };
    enum { SIZE = 0xFFFF };

    my_holdings_t() : emb::server_holdings_t(START, SIZE) {

        regs.resize(SIZE);
        memset(&regs[0], 0, SIZE*2);
    }

    uint8_t on_read_regs(emb_const_pdu_t* _req, uint16_t _offset,
                         uint16_t _quantity, uint16_t* _pvalues) {

        memcpy(_pvalues, &regs[_offset], _quantity*2);

        printf("Read Holdings: start:0x%04X, length:0x%04X\n", _offset, _quantity);
        return 0;
    }

    uint8_t on_write_regs(emb_const_pdu_t* _req, uint16_t _offset,
                          uint16_t _quantity, const uint16_t* _pvalues) {

        printf("Write Holdings: start:0x%04X, length:0x%04X\nData:", _offset, _quantity);
        for(int i=0; i<_quantity; ++i)
            printf("0x%04X ", _pvalues[i]);
        printf("\n");

        memcpy(&regs[_offset], _pvalues, _quantity*2);

        return 0;
    }
private:
    std::vector<uint16_t> regs;
};

class my_file_t : public emb::server_file_t {
public:
    enum { FILENO = 0 };
    enum { START = 0x0000 };
    enum { SIZE = 0xFFFF };

    my_file_t() : emb::server_file_t(FILENO/*, START, SIZE*/) {

        regs.resize(SIZE);
        memset(&regs[0], 0, SIZE*2);
    }

    uint8_t on_read_file(uint16_t _offset,
                         uint16_t _quantity, uint16_t* _pvalues) {

        memcpy(_pvalues, &regs[_offset], _quantity*2);

        printf("Read File: start:0x%04X, length:0x%04X\n", _offset, _quantity);
        return 0;
    }

    uint8_t on_write_file(uint16_t _offset,
                          uint16_t _quantity, const uint16_t* _pvalues) {

        printf("Write File: start:0x%04X, length:0x%04X\nData:", _offset, _quantity);
        for(int i=0; i<_quantity; ++i)
            printf("0x%04X ", _pvalues[i]);
        printf("\n");

        memcpy(&regs[_offset], _pvalues, _quantity*2);

        return 0;
    }
private:
    std::vector<uint16_t> regs;
};

emb_debug_helper_t emb_debug_helper;

int main(int argc, char* argv[]) {

    printf("emodbus server test\n");

    emb::super_server_t ssrv;

    struct event_base *base = event_base_new();

    posix_serial_rtu_t psp(base, "/dev/ttyUSB0", 115200);

    ssrv.set_proto(psp.get_proto());

    psp.get_proto()->flags |= EMB_PROTO_FLAG_DUMD_PAKETS;

    emb_debug_helper.enable_dumping();

    sleep(1);

    my_holdings_t h;
    my_file_t f;

    emb::server_t srv1(16);

    srv1.add_function(0x03, emb_srv_read_holdings);
    srv1.add_function(0x06, emb_srv_write_reg);
    srv1.add_function(0x10, emb_srv_write_regs);
    srv1.add_function(0x16, emb_srv_mask_reg);

    srv1.add_function(0x14, emb_srv_read_file);
    srv1.add_function(0x15, emb_srv_write_file);

    srv1.add_holdings(h);
    srv1.add_file(f);

    ssrv.add_server(srv1);

    event_base_dispatch(base);

    return 0;
}
