
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <errno.h>

#include <emodbus/client/client.h>
#include <emodbus/protocols/rtu.h>
#include <emodbus/client/read_holding_regs.h>
#include <emodbus/client/write_multi_regs.h>
#include <emodbus/base/add/container_of.h>
#include <emodbus/base/modbus_errno.h>

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

class emodbus_sync_client_t {
public:
    emodbus_sync_client_t() {

        client.user_data = this;

        emb_client_init(&client);
        emb_client_add_function(&client, &read_holding_regs_interface);
        emb_client_add_function(&client, &write_multi_regs_interface);

        pthread_mutex_init(&mutex, NULL);
        pthread_mutex_trylock(&mutex);
    }

    int do_request_sync(int _server_addr,
                        unsigned int _timeout,
                        emb_const_pdu_t* _request,
                        emb_pdu_t *_response) {

        struct timespec expiry_time;
        int res;

        req.req_pdu = _request;
        req.resp_pdu = _response;
        req.procs = &procs;
        req.user_data = this;

        if((res = emb_client_do_request(&client, _server_addr, &req)) != 0) {
            return res;
        }

        timespec_get_clock_realtime(&expiry_time);
        timespec_add_ms(&expiry_time, _timeout);
        res = pthread_mutex_timedlock(&mutex, &expiry_time);

        if(!res) {
            return result;
        }
        else if(res == ETIMEDOUT) {
            emb_client_wait_timeout(&client);
            return -modbus_resp_timeout;
        }
        else {
            return res;
        }
    }

    void set_proto(struct emb_protocol_t* _proto) {
        emb_client_set_proto(&client, _proto);
    }

private:

    static void emb_on_response(struct emb_client_request_t* _req, int _slave_addr) {
        emodbus_sync_client_t* _this = (emodbus_sync_client_t*)_req->user_data;
        _this->result = 0;
        pthread_mutex_unlock(&_this->mutex);
    }

    static void emb_on_error(struct emb_client_request_t* _req, int _errno) {
        emodbus_sync_client_t* _this = (emodbus_sync_client_t*)_req->user_data;
        _this->result = _errno;
        pthread_mutex_unlock(&_this->mutex);
    }

    pthread_mutex_t mutex;

    struct emb_client_request_t req;
    static struct emb_client_req_procs_t procs;

    struct emb_client_t client;

    int result;

} mb_client;

struct emb_client_req_procs_t emodbus_sync_client_t::procs = {
    emodbus_sync_client_t::emb_on_response,
    emodbus_sync_client_t::emb_on_error
};

class emodbus_client_t {
public:
    emodbus_client_t() {

        client.user_data = this;

        emb_client_init(&client);
        emb_client_add_function(&client, &read_holding_regs_interface);
        emb_client_add_function(&client, &write_multi_regs_interface);

        pthread_mutex_init(&mutex, NULL);
        pthread_mutex_trylock(&mutex);
    }

    int do_request(int _server_addr,
                   unsigned int _timeout,
                   emb_const_pdu_t* _request,
                   emb_pdu_t *_response) {

        struct timespec expiry_time;
        int res;

        req.req_pdu = _request;
        req.resp_pdu = _response;
        req.procs = &procs;
        req.user_data = this;

        res = emb_client_do_request(&client, _server_addr, &req);

        return res;
    }

    void set_proto(struct emb_protocol_t* _proto) {
        emb_client_set_proto(&client, _proto);
    }

protected:

    virtual void emb_client_on_response(struct emb_client_request_t* _req, int _slave_addr) { }
    virtual void emb_client_on_error(struct emb_client_request_t* _req, int _errno) { }

private:

    static void emb_on_response(struct emb_client_request_t* _req, int _slave_addr) {
        emodbus_client_t* _this = (emodbus_client_t*)_req->user_data;
        _this->emb_client_on_response(_req, _slave_addr);
    }

    static void emb_on_error(struct emb_client_request_t* _req, int _errno) {
        emodbus_client_t* _this = (emodbus_client_t*)_req->user_data;
        _this->emb_client_on_error(_req, _errno);
    }

    pthread_mutex_t mutex;

    struct emb_client_request_t req;
    static struct emb_client_req_procs_t procs;

    struct emb_client_t client;
};

struct emb_client_req_procs_t emodbus_client_t::procs = {
    emodbus_client_t::emb_on_response,
    emodbus_client_t::emb_on_error
};


void* thr_proc(void* p) {

    emodbus_sync_client_t* client = (emodbus_sync_client_t*)p;

    struct event_base *base = event_base_new();

    posix_serial_port_rtu_t psp(base, "/dev/ttyUSB0", 115200);

    struct posix_serial_port_t serial_port;

    client->set_proto(psp.get_proto());

    event_base_dispatch(base);

    posix_serial_port_close(&serial_port);
}

class pdu_t : public emb_pdu_t {
public:
    pdu_t() {
        buffer.resize(128);
        max_size = buffer.size();
        emb_pdu_t::data = &buffer[0];
    }

    operator emb_const_pdu_t* () const {
        return (emb_const_pdu_t*) (this);
    }

private:
    std::vector<char> buffer;
};

int main(int argc, char* argv[]) {

    int res;

    printf("emodbus sync client test\n");

    pthread_t pthr;

    pthread_create(&pthr, NULL, thr_proc, (void*)&mb_client);

    sleep(1);

    pdu_t reqa8, reqd8;
    pdu_t ans;

    read_holding_regs_make_req(&reqa8, 0x0000, 8);
    read_holding_regs_make_req(&reqd8, 0x0000, 1);

    for(int i=0; i<100; ++i) {

        res = mb_client.do_request_sync(16, 100, reqa8, &ans);
        if(res)
            printf("Error: %d \"%s\"\n", res, emb_strerror(-res));

        usleep(1000*1);

        res = mb_client.do_request_sync(48, 100, reqd8, &ans);
        if(res)
            printf("Error: %d \"%s\"\n", res, emb_strerror(-res));


        usleep(1000*10);
        //printf("---------------> do_request() := %d\n", res);
    }

    pthread_join(pthr, NULL);

    return 0;
}
