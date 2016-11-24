
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <list>
#include <errno.h>
#include <string.h>
#include <bitset>
#include <signal.h>

#include <stdlib.h>

#include <emodbus/emodbus.hpp>
#include <emodbus/base/common.h>
#include <emodbus/base/modbus_errno.h>
#include <emodbus/server/server.h>
#include <emodbus/server/bits.h>
#include <emodbus/server/regs.h>
#include <emodbus/server/file.h>

#include <emodbus/impl/posix/tcp-client-rtu.hpp>
#include <emodbus/impl/posix/mb-rtu-via-serial.h>
#include <emodbus/impl/posix/mb-tcp-via-tcp-client.h>
#include <emodbus/impl/posix/mb-tcp-via-tcp-server.h>
#include <emodbus/impl/posix/dumper.h>

#include <event2/event.h>

#include "timespec_operations.h"

class my_coils_t : public emb::server::coils_t {
public:
    my_coils_t(uint16_t _start, uint16_t _size) :
        emb::server::coils_t(_start, _size) {
        values.resize(_size);
        for(int i=0; i<_size; ++i) {
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

class my_discrete_inputs_t : public emb::server::discrete_inputs_t {
public:
    my_discrete_inputs_t(uint16_t _start, uint16_t _size) :
        emb::server::discrete_inputs_t(_start, _size) {
        values.resize(_size);
        for(int i=0; i<_size; ++i) {
            values[i] = false;
        }
    }

private:
    uint8_t on_read_bits(uint16_t _offset,
                         uint16_t _quantity,
                         uint8_t* _pvalues) {
        for(int byte=0; _quantity != 0; ++byte) {
            _pvalues[byte] = 0;
            const int n_bits = _quantity > 8 ? 8 : _quantity;
            for(int bit=0; bit<n_bits; ++bit) {
                if(values[_offset + byte*8 + bit]) {
                    _pvalues[byte] |= (1 << bit);
                }
            }
            _quantity -= n_bits;
        }
        return 0;
    }

    std::vector<bool> values;
};

class my_holdings_t : public emb::server::holding_regs_t {
public:

    my_holdings_t(uint16_t _start, uint16_t _size) :
        emb::server::holding_regs_t(_start, _size) {

        regs_.resize(_size);
        memset(&regs_[0], 0, _size*2);
    }

    uint8_t on_read_regs(uint16_t _offset,
                         uint16_t _quantity, uint16_t* _pvalues) {

        memcpy(_pvalues, &regs_[_offset], _quantity*2);

//        printf("Read Holdings: start:0x%04X, length:0x%04X\n", _offset, _quantity);
//        fflush(stdout);
        return 0;
    }

    uint8_t on_write_regs(uint16_t _offset,
                          uint16_t _quantity, const uint16_t* _pvalues) {

//        printf("Write Holdings: start:0x%04X, length:0x%04X\nData:", _offset, _quantity);
//        for(int i=0; i<_quantity; ++i)
//            printf("0x%04X ", _pvalues[i]);
//        printf("\n");
//        fflush(stdout);

        memcpy(&regs_[_offset], _pvalues, _quantity*2);

        return 0;
    }

    const std::vector<uint16_t>& regs() const
    { return regs_; }

    std::vector<uint16_t> getRegs() const;
    void setRegs(const std::vector<uint16_t> &value);

private:
    std::vector<uint16_t> regs_;
};

class my_input_regs_t : public emb::server::input_regs_t {
public:

    my_input_regs_t(uint16_t _start, uint16_t _size) :
        emb::server::input_regs_t(_start, _size) {
        regs.resize(_size);
        for(int i=0; i<_size; ++i)
            regs[i] = _start + i;
    }

    uint8_t on_read_regs(uint16_t _offset,
                         uint16_t _quantity, uint16_t* _pvalues) {
        memcpy(_pvalues, &regs[_offset], _quantity*2);
        return 0;
    }

    uint8_t on_write_regs(uint16_t _offset,
                          uint16_t _quantity, const uint16_t* _pvalues) {
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
        return 0;
    }

    uint8_t on_write_file(uint16_t _offset,
                          uint16_t _quantity, const uint16_t* _pvalues) {
        memcpy(&regs[_offset], _pvalues, _quantity*2);
        return 0;
    }
private:
    std::vector<uint16_t> regs;
};

//serial_rtu_t rtu;
//tcp_client_tcp_t tcp;

template <typename T>
bool is_addr_belongs_to(const T& _range, uint16_t _addr)
{
   return (_range.start() <= _addr && _addr < (_range.start() + _range.size()));
}

class my_server_t : public emb::server::server_t {
public:
    my_server_t(int _address)
        : emb::server::server_t(_address)
        , di1(0x0000, 0x8000)
        , di2(0xE800, 0x1800)
        , coils1(0x0000, 0x7FED)
        , coils2(0x7FED, 0x0077)
        , coils3(0x8065, 0x0001)
        , coils4(0xE800, 0x1800)
        , holdings1(0x0000, 0x7FED)
        , holdings2(0x7FED, 0x0077)
        , holdings3(0x8065, 0x0001)
        , holdings4(0xE800, 0x1800)
        , inputs1(0x0000, 0x8000)
        , inputs2(0xE800, 0x1800)
    {
        add_function(0x01, emb_srv_read_bits);
        add_function(0x05, emb_srv_write_coil);
        add_function(0x0F, emb_srv_write_coils);

        add_function(0x02, emb_srv_read_bits);

        add_function(0x03, emb_srv_read_regs);
        add_function(0x06, emb_srv_write_reg);
        add_function(0x10, emb_srv_write_regs);
        add_function(0x16, emb_srv_mask_reg);

        add_function(0x04, emb_srv_read_regs);

        add_function(0x14, emb_srv_read_file);
        add_function(0x15, emb_srv_write_file);

        add_function(0x17, emb_srv_read_write_regs);

        add_function(0x18, emb_srv_read_fifo);

        if(!add_discrete_inputs(di1))
            printf("Error with add_discrete_inputs(di1)\n");
        if(!add_discrete_inputs(di2))
            printf("Error with add_discrete_inputs(di2)\n");

        if(!add_coils(coils1))
            printf("Error with add_coils(coils1)\n");
        if(!add_coils(coils2))
            printf("Error with add_coils(coils2)\n");
        if(!add_coils(coils3))
            printf("Error with add_coils(coils3)\n");
        if(!add_coils(coils4))
            printf("Error with add_coils(coils4)\n");

        if(!add_holding_regs(holdings1))
            printf("Error with add_holding_regs(holdings1)\n");
        if(!add_holding_regs(holdings2))
            printf("Error with add_holding_regs(holdings2)\n");
        if(!add_holding_regs(holdings3))
            printf("Error with add_holding_regs(holdings3)\n");
        if(!add_holding_regs(holdings4))
            printf("Error with add_holding_regs(holdings4)\n");

        if(!add_input_regs(inputs1))
            printf("Error with add_input_regsinputs1(inputs1)\n");
        if(!add_input_regs(inputs2))
            printf("Error with add_input_regsinputs1(inputs2)\n");

        if(!add_file(file))
            printf("Error with add_file(file)\n");
    }

    virtual uint8_t on_read_fifo(uint16_t _address,
                                 uint16_t* _fifo_buf, uint8_t* _fifo_count)
    {

        const my_holdings_t* regs = NULL;

        if(is_addr_belongs_to(holdings1, _address))
            regs = &holdings1;
        else if(is_addr_belongs_to(holdings2, _address))
            regs = &holdings2;
        else if(is_addr_belongs_to(holdings3, _address))
            regs = &holdings3;
        else if(is_addr_belongs_to(holdings4, _address))
            regs = &holdings4;

        // just for testing the exception reaction
        if(0xABC0 <= _address && _address <= 0xABCF)
            return MBE_ILLEGAL_DATA_ADDR;

        if(!regs) {
            *_fifo_count = 0;
            return 0;
        }

        _address -= regs->start();

        size_t sz = regs->size() - _address;

        if(sz > EMB_SRV_READ_FIFO_MAX_REGS)
            sz = EMB_SRV_READ_FIFO_MAX_REGS;

        memcpy(_fifo_buf, regs->regs().data()+_address, sz*2);

        *_fifo_count = sz;

        return 0;
    }

private:
    my_discrete_inputs_t di1,di2;
    my_coils_t coils1,coils2,coils3,coils4;
    my_holdings_t holdings1,holdings2,holdings3,holdings4;
    my_input_regs_t inputs1,inputs2;
    my_file_t file;
};

static void signal_cb(evutil_socket_t sig, short events, void *user_data) {
    struct event_base *base = (struct event_base*)user_data;
    printf("Caught an interrupt signal; exiting cleanly.\n");
    event_base_loopexit(base, NULL);
}

enum { SERVERS_FIRST_ADDRESS = 1 };
enum { SERVERS_LAST_ADDRESS = 255 };
enum { SERVERS_NUMBER = SERVERS_LAST_ADDRESS - SERVERS_FIRST_ADDRESS };

int main(int argc, char* argv[]) {

    printf("emodbus server test\n");

    struct event *signal_event;

    emb::server::super_server_t ssrv;

    struct event_base *base = event_base_new();

    signal_event = evsignal_new(base, SIGINT, signal_cb, (void *)base);
    if (!signal_event || event_add(signal_event, NULL)<0) {
        fprintf(stderr, "Could not create/add a signal event!\n");
        fflush(stderr);
    }

    //res = rtu.open(base, "/dev/ttyUSB0", 115200);
    //res = tcp.open(base, "127.0.0.1", 9992);
    emb_tcp_via_tcp_server_t* tcp = emb_tcp_via_tcp_server_create(base, 8502);

    //struct emb_rtu_via_serial_t* rtu = emb_rtu_via_serial_create(base, 5, "/dev/pts/26", 1152000);

    if(!tcp)
        exit(1);

    //ssrv.set_proto(tcp/*rtu*/.get_proto());
    ssrv.set_transport(emb_tcp_via_tcp_server_get_transport(tcp));

    //tcp/*rtu*/.get_proto()->flags |= EMB_PROTO_FLAG_DUMD_PAKETS;
    emb_tcp_via_tcp_server_get_transport(tcp)->flags |= EMB_TRANSPORT_FLAG_DUMD_PAKETS;
    emb_posix_dumping_stream = stdout;
//    emb_posix_dumper_enable_rx_tx();

    sleep(1);

    std::vector<my_server_t*> servers;

    printf("Creating %d servers ...\n", SERVERS_NUMBER);
    for(int i=SERVERS_FIRST_ADDRESS; i<=SERVERS_LAST_ADDRESS; ++i) {
        my_server_t* srv = new my_server_t(i);
        servers.push_back(srv);
        ssrv.add_server(*srv);
        printf("srv(%d) addr: %p\n", i, srv);
        fflush(stdout);
    }
    printf("OK, Starting of the dispatcher\n");

    event_base_dispatch(base);

    printf("Cleaning ...\n");

    for(int i=0; i<servers.size(); ++i) {
        if(servers[i]) {
            printf("srv(%d) addr: %p\n", i+SERVERS_FIRST_ADDRESS, servers[i]);
            fflush(stdout);
            delete servers[i];
        }
    }

    printf("OK, Buy.\n");

    return 0;
}
