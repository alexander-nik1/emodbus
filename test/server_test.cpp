
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <list>
#include <errno.h>

#include <emodbus/emodbus.hpp>

#include <emodbus/client/client.h>
#include <emodbus/base/common.h>
#include <emodbus/protocols/rtu.h>
#include <emodbus/base/add/container_of.h>
#include <emodbus/base/modbus_errno.h>


#include <emodbus/server/server.h>
#include <emodbus/server/holdings.h>

#include <emodbus/base/add/stream.h>
#include "posix-serial-port.h"

#include <pthread.h>

#include <event2/event.h>

#include "timespec_operations.h"

class posix_serial_port_rtu_t {
public:
    posix_serial_port_rtu_t(struct event_base *_base,
                            const char* _dev_name,
                            unsigned int _baudrate) {

        posix_serial_port_open(&posix_serial_port, _base, _dev_name, _baudrate);

        rx_buffer.resize(128);
        tx_buffer.resize(128);

        modbus_rtu.user_data = this;
        modbus_rtu.rx_buffer = &rx_buffer[0];
        modbus_rtu.tx_buffer = &tx_buffer[0];
        modbus_rtu.rx_buf_size = rx_buffer.size();
        modbus_rtu.tx_buf_size = tx_buffer.size();
        modbus_rtu.modbus_rtu_on_char = modbus_rtu_on_char;

        modbus_rtu_initialize(&modbus_rtu);

        stream_connect(&posix_serial_port.output_stream, &modbus_rtu.input_stream);
        stream_connect(&modbus_rtu.output_stream, &posix_serial_port.input_stream);

        char_pause.tv_sec = 0;
      //  enum { pause = 100 };
        char_pause.tv_usec = 1000 * 10; //(1000 * 1000) / (_baudrate / pause);

        char_timeout_timer = event_new(_base, -1, EV_TIMEOUT/* | EV_PERSIST*/, on_timer, this);
    }

    ~posix_serial_port_rtu_t() {

        event_del(char_timeout_timer);
        event_free(char_timeout_timer);

        posix_serial_port_close(&posix_serial_port);
    }

    struct emb_protocol_t* get_proto() {
        return &modbus_rtu.proto;
    }

private:
    static void modbus_rtu_on_char(void* _user_data) {
        posix_serial_port_rtu_t* _this = (posix_serial_port_rtu_t*)_user_data;
        event_add(_this->char_timeout_timer, &_this->char_pause);
    }

    static void on_timer(evutil_socket_t fd, short what, void *arg) {
        posix_serial_port_rtu_t* _this = (posix_serial_port_rtu_t*)arg;
        modbus_rtu_on_char_timeout(&_this->modbus_rtu);
    }

    std::vector<unsigned char> rx_buffer, tx_buffer;

private:
    struct posix_serial_port_t posix_serial_port;
    struct modbus_rtu_t modbus_rtu;
    struct timeval char_pause;
    struct event *char_timeout_timer;
};

class emodbus_server_t;

class server_holdings_t {
    friend class emodbus_server_t;
public:

    server_holdings_t() {
        set_funcs();
    }

    server_holdings_t(uint16_t _start, uint16_t _size) {
        h.start = _start;
        h.size = _size;
        set_funcs();
    }

    void set_start(uint16_t _start)
    { h.start = _start; }

    void set_size(uint16_t _size)
    { h.size = _size; }

    virtual uint8_t on_read_regs(emb_const_pdu_t* _req,
                                 uint16_t _offset,
                                 uint16_t _quantity,
                                 uint16_t* _pvalues)
    { return 0; }

    virtual uint8_t on_write_regs(emb_const_pdu_t* _req,
                                  uint16_t _offset,
                                  uint16_t _quantity,
                                  const uint16_t* _pvalues)
    { return 0; }

private:

    void set_funcs() {
        h.read_regs = read_regs;
        h.write_regs = write_regs;
    }

    static uint8_t read_regs(struct emb_srv_holdings_t* _rr,
                         emb_const_pdu_t* _req,
                         uint16_t _offset,
                         uint16_t _quantity,
                         uint16_t* _pvalues) {
        server_holdings_t* _this = container_of(_rr, server_holdings_t, h);
        return _this->on_read_regs(_req, _offset, _quantity, _pvalues);
    }

    static uint8_t write_regs(struct emb_srv_holdings_t* _rr,
                          emb_const_pdu_t* _req,
                          uint16_t _offset,
                          uint16_t _quantity,
                          const uint16_t* _pvalues) {
        server_holdings_t* _this = container_of(_rr, server_holdings_t, h);
        return _this->on_write_regs(_req, _offset, _quantity, _pvalues);
    }

    struct emb_srv_holdings_t h;
};

class emodbus_sserver_t;

class emodbus_server_t {
    friend class emodbus_sserver_t;

public:
    emodbus_server_t(int _addr) : addr_(_addr) {
        srv.get_function = get_function;
        srv.get_holdings = get_holdings;
    }

    int addr() const { return addr_; }

private:
    static emb_srv_function_t get_function(struct emb_server_t* _srv, uint8_t _func) {
        emodbus_server_t* _this = container_of(_srv, emodbus_server_t, srv);
        for(func_iter i=_this->functions.begin(); i != _this->functions.end(); ++i) {
            if(i->func_no == _func)
                return i->func;
        }
        return nullptr;
    }

    static struct emb_srv_holdings_t* get_holdings(struct emb_server_t* _srv, uint16_t _begin) {
        emodbus_server_t* _this = container_of(_srv, emodbus_server_t, srv);
        for(holdnigs_iter i=_this->holdings.begin(); i != _this->holdings.end(); ++i) {
            if((i->h.start <= _begin) && ((i->h.start + i->h.size) < _begin))
                return &(i->h);
        }
        return nullptr;
    }

public:
    struct function_t {
        uint8_t func_no;
        emb_srv_function_t func;
    };

    typedef std::vector<function_t>::iterator func_iter;
    std::vector<function_t> functions;

    typedef std::list<server_holdings_t>::iterator holdnigs_iter;
    std::list<server_holdings_t> holdings;

private:
    int addr_;
    struct emb_server_t srv;
};

class emodbus_sserver_t {
public:

    emodbus_sserver_t() {
        ssrv.get_server = _get_server;

        rx_pdu.resize(128);
        tx_pdu.resize(128);

        ssrv.rx_pdu = &rx_pdu;
        ssrv.tx_pdu = &tx_pdu;
    }

    void set_proto(struct emb_protocol_t* _proto) {
        emb_super_server_set_proto(&ssrv, _proto);
    }

private:
    static struct emb_server_t* _get_server(struct emb_super_server_t* _ssrv,
                                            uint8_t _address) {
        emodbus_sserver_t* _this = container_of(_ssrv, emodbus_sserver_t, ssrv);
        for(srv_iter i=_this->servers.begin(); i != _this->servers.end(); ++i) {
            if(i->addr() == _address)
                return &i->srv;
        }
        return nullptr;
    }
public:
    typedef std::list<emodbus_server_t>::iterator srv_iter;
    std::list<emodbus_server_t> servers;
private:
    struct emb_super_server_t ssrv;
    emb::pdu_t rx_pdu,tx_pdu;

} mb_server;

void* thr_proc(void* p) {

    emodbus_sserver_t* server = (emodbus_sserver_t*)p;

    struct event_base *base = event_base_new();

    posix_serial_port_rtu_t psp(base, "/dev/ttyUSB0", 115200);

    server->set_proto(psp.get_proto());

    psp.get_proto()->flags |= EMB_PROTO_FLAG_DUMD_PAKETS;

    event_base_dispatch(base);
}

void print_all_read_file_answer_data(emb_const_pdu_t* ans);
void write_and_read_file_record_test();

int main(int argc, char* argv[]) {

    int res,i;

    printf("emodbus sync client test\n");

    pthread_t pthr;

    pthread_create(&pthr, NULL, thr_proc, (void*)&mb_server);

    sleep(1);

    emodbus_server_t srv1(16);

    mb_server.servers.push_back(srv1);

    pthread_join(pthr, NULL);

    return 0;
}
