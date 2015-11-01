
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>

#include "../client/sync_client.h"
#include "../protocols/rtu.h"
#include "../client/read_holding_regs.h"
#include "../client/write_multi_regs.h"
#include "../base/add/container_of.h"
#include "../base/modbus_errno.h"

#include "../base/add/stream.h"
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
        char_pause.tv_usec = 1000 * 1; //(1000 * 1000) / (_baudrate / pause);

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
        //printf("%s\n", __PRETTY_FUNCTION__);
        event_add(_this->char_timeout_timer, &_this->char_pause);
    }

    static void on_timer(evutil_socket_t fd, short what, void *arg) {
        posix_serial_port_rtu_t* _this = (posix_serial_port_rtu_t*)arg;
        //printf("%s\n", __PRETTY_FUNCTION__);
        modbus_rtu_on_char_timeout(&_this->modbus_rtu);
    }

    std::vector<unsigned char> rx_buffer, tx_buffer;

private:
    struct posix_serial_port_t posix_serial_port;
    struct modbus_rtu_t modbus_rtu;
    struct timeval char_pause;
    struct event *char_timeout_timer;
};

void* thr_proc(void* p) {

    struct emb_sync_client_t* client = (struct emb_sync_client_t*)p;

    struct event_base *base = event_base_new();

    posix_serial_port_rtu_t psp(base, "/dev/ttyUSB0", 115200);

    struct posix_serial_port_t serial_port;

    emb_sync_client_set_proto(client, psp.get_proto());

    event_base_dispatch(base);

    posix_serial_port_close(&serial_port);
}

class mb_client_t {
public:
    mb_client_t() {

        client.resp_timeout_mutex.user_data = this;
        client.resp_timeout_mutex.lock_timeout = mutex_lock_timeout;
        client.resp_timeout_mutex.unlock = mutex_unlock;

        emb_sync_client_initialize(&client);
        emb_sync_client_add_function(&client, 0x03, &read_holding_regs_interface);
        emb_sync_client_add_function(&client, 0x10, &write_multi_regs_interface);

        pthread_mutex_init(&mutex, NULL);
        pthread_mutex_trylock(&mutex);
    }

    int do_request(int _server_addr,
                   unsigned int _timeout,
                   emb_const_pdu_t* _request,
                   emb_const_pdu_t **_response) {

        return emb_sync_client_do_request(&client, _server_addr, _timeout, _request, _response);
    }

private:
    static int mutex_lock_timeout(void* _user_data, unsigned int _timeout) {
        //printf("%s\n", __PRETTY_FUNCTION__);
        mb_client_t* _this = (mb_client_t*)_user_data;
        struct timespec expiry_time;
        timespec_get_clock_realtime(&expiry_time);
        timespec_add_ms(&expiry_time, _timeout);
        pthread_mutex_timedlock(&_this->mutex, &expiry_time);
    }

    static void mutex_unlock(void* _user_data) {
        //printf("%s\n", __PRETTY_FUNCTION__);
        mb_client_t* _this = (mb_client_t*)_user_data;
        pthread_mutex_unlock(&_this->mutex);
    }

    //std::timed_mutex mutex;
    pthread_mutex_t mutex;

public:
    struct emb_sync_client_t client;

} mb_client;

class pdu_t : public emb_pdu_t {
public:
    pdu_t() {
        buffer.resize(128);
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

    pthread_create(&pthr, NULL, thr_proc, (void*)&mb_client.client);

    sleep(1);

    pdu_t reqa8, reqd8;
    emb_const_pdu_t* ans;

    read_holding_regs_make_req(&reqa8, 0x0000, 8);
    read_holding_regs_make_req(&reqd8, 0x0000, 1);

    for(int i=0; i<10000; ++i) {
        usleep(1000*1);
        //printf("---------------> do_request()\n");

        res = mb_client.do_request(16, 300, reqa8, &ans);
        if(res)
            printf("Error: %d \"%s\"\n", res, emb_strerror(-res));

        usleep(1000*1);
        res = mb_client.do_request(48, 300, reqd8, &ans);
        if(res)
            printf("Error: %d \"%s\"\n", res, emb_strerror(-res));

        //printf("---------------> do_request() := %d\n", res);
    }

    pthread_join(pthr, NULL);

    return 0;
}
