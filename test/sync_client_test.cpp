
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <errno.h>

#include <emodbus/emodbus.hpp>

#include <emodbus/client/client.h>
#include <emodbus/base/common.h>
#include <emodbus/protocols/rtu.h>
#include <emodbus/base/add/container_of.h>
#include <emodbus/base/modbus_errno.h>


#include <emodbus/client/read_holding_regs.h>
#include <emodbus/client/write_mask_reg.h>
#include <emodbus/client/write_multi_regs.h>
#include <emodbus/client/write_single_reg.h>
#include <emodbus/client/read_file_record.h>
#include <emodbus/client/write_file_record.h>
#include <emodbus/client/read_fifo.h>

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

class emodbus_sync_client_t : public emb::sync_client_t {
public:
    emodbus_sync_client_t() {
        pthread_mutex_init(&mutex, NULL);
        pthread_mutex_trylock(&mutex);
    }

private:

    int emb_client_start_wait(unsigned int _timeout) {
        struct timespec expiry_time;
        int res;
        timespec_get_clock_realtime(&expiry_time);
        timespec_add_ms(&expiry_time, _timeout);
        res = pthread_mutex_timedlock(&mutex, &expiry_time);

        if(res == ETIMEDOUT) {
            return 1;
        }
        else {
            return res;
        }
    }

    void emb_client_stop_wait() {
        pthread_mutex_unlock(&mutex);
    }

    pthread_mutex_t mutex;

} mb_client;

void* thr_proc(void* p) {

    emodbus_sync_client_t* client = (emodbus_sync_client_t*)p;

    struct event_base *base = event_base_new();

    posix_serial_port_rtu_t psp(base, "/dev/ttyUSB0", 115200);

    client->set_proto(psp.get_proto());

    psp.get_proto()->flags |= EMB_PROTO_FLAG_DUMD_PAKETS;

    event_base_dispatch(base);
}

void print_all_read_file_answer_data(emb_const_pdu_t* ans);
void write_and_read_file_record_test();

int main(int argc, char* argv[]) {

    int res,i;

    printf("emodbus sync client test\n");

    pthread_t pthr;

    pthread_create(&pthr, NULL, thr_proc, (void*)&mb_client);

    sleep(1);


    emb::pdu_t req(emb_read_fifo_calc_req_data_size());

    emb::pdu_t ans(emb_read_fifo_calc_answer_data_size());

    res = emb_read_fifo_make_req(req, 0x0000);
    printf("emb_read_fifo_make_req = %d\n", res);

    for(i=0; i<1000; ++i) {

        res = mb_client.do_request(224, 100, req, ans);
        if(res)
            printf("Error: %d \"%s\"\n", res, emb_strerror(-res));

        int q = emb_read_fifo_regs_count(ans);

        for(int j=0; j<q; ++j) {
            printf("%04X ", emb_read_fifo_get_data(ans, j));
        }

        printf("\n");

        usleep(1000*5000);
        //printf("---------------> do_request() := %d\n", res);
    }

    pthread_join(pthr, NULL);

    return 0;
}

void print_all_read_file_answer_data(emb_const_pdu_t* ans) {

    for(emb_read_file_subansw_t* sa = emb_read_file_first_subanswer(ans);
        sa != NULL; sa = emb_read_file_next_subanswer(ans, sa)) {

        const uint16_t q = emb_read_file_subanswer_quantity(sa);

        printf("Subanswer:(%d): ", q);

        for(int j=0; j<q; ++j) {
            printf("%04X ", emb_read_file_subanswer_data(sa, j));
        }

        printf("\n");
    }
}

void write_and_read_file_record_test() {

    int res;

    emb_read_file_req_t reqs[1];

    reqs[0].file_number = 0;
    reqs[0].record_number = 0x0000;
    reqs[0].record_length = 0x0011;

    enum { wr_sz = 3 };

    emb_write_file_req_t rew[wr_sz];

    const uint16_t dw[wr_sz][3] = {
        { 0x1111, 0xDEAD, 0xC0FE },
        { 0x2222, 0xDEAD, 0xC0FE },
        { 0x3333, 0xDEAD, 0xC0FE }
    };

    for(int i=0; i<wr_sz; ++i) {
        rew[i].file_number = 0;
        rew[i].record_number = i * 4;
        rew[i].record_length = 3;
        rew[i].data = dw[i];
    }

    emb::pdu_t req(emb_read_file_calc_req_data_size(1));

    emb::pdu_t ans(emb_read_file_calc_answer_data_size(reqs, 1));

    emb::pdu_t reqw(emb_write_file_calc_req_data_size(rew, wr_sz));

    emb::pdu_t answ(emb_write_file_calc_answer_data_size(rew, wr_sz));

    printf("size = %d\n", reqw.max_size);

    res = emb_read_file_make_req(req, reqs, 1);
    printf("emb_read_file_make_req = %d\n", res);

    res = emb_write_file_make_req(reqw, rew, wr_sz);
    printf("emb_write_file_make_req = %d\n", res);

    for(int i=0; i<10; ++i) {

        res = mb_client.do_request(16, 100, reqw, answ);
        if(res)
            printf("Error: %d \"%s\"\n", res, emb_strerror(-res));

        res = mb_client.do_request(16, 100, req, ans);
        if(res)
            printf("Error: %d \"%s\"\n", res, emb_strerror(-res));

        print_all_read_file_answer_data(ans);

        usleep(1000*10);
        //printf("---------------> do_request() := %d\n", res);
    }
}
