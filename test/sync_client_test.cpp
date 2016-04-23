
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <errno.h>
#include <stdlib.h>

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

#include <pthread.h>

#include <event2/event.h>

#include "timespec_operations.h"

#include "posix_serial_rtu/posix_serial_rtu.hpp"
#include "dumping_helper.hpp"

class cleent_t : public emb::client::client_t {
public:
    cleent_t() {
        pthread_mutex_init(&mutex, NULL);
        pthread_mutex_trylock(&mutex);

        emb::client::client_t::set_sync(true);
    }

private:

    int emb_sync_client_start_wait(unsigned int _timeout) {
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

    void emb_on_response(int _slave_addr) {
        pthread_mutex_unlock(&mutex);
    }

    void emb_on_error(int _slave_addr, int _errno) {
        pthread_mutex_unlock(&mutex);
    }

    pthread_mutex_t mutex;

} mb_client;

emb_debug_helper_t emb_debug_helper;

void* thr_proc(void* p) {

    cleent_t* client = (cleent_t*)p;

    struct event_base *base = event_base_new();

    posix_serial_rtu_t psp(base, "/dev/ttyUSB1", 115200);

    client->set_proto(psp.get_proto());

    psp.get_proto()->flags &= ~EMB_PROTO_FLAG_DUMD_PAKETS;

    emb_debug_helper.enable_dumping();

    event_base_dispatch(base);
}

void print_all_read_file_answer_data(emb_const_pdu_t* ans);
void write_and_read_file_record_test();
void coils_test();
void registers_test();

int main(int argc, char* argv[]) {

    printf("emodbus sync client test\n");

    pthread_t pthr;

    pthread_create(&pthr, NULL, thr_proc, (void*)&mb_client);

    sleep(1);

    coils_test();

    pthread_join(pthr, NULL);

    return 0;
}

void print_boolean_array(const bool* _arr, size_t _arr_size) {
    for(size_t i=0; i<_arr_size; ++i) {
        printf("%d ", _arr[i]);
    }
}

bool cmp_boolean_arrays(const bool* _a, const bool* _b, size_t _arr_size) {
    for(size_t i=0; i<_arr_size; ++i) {
        if(_a[i] != _b[i])
            return false;
    }
    return true;
}

void coils_test() {

    int res,i, errors=0;

    emb::client::read_coils_t rr;
    emb::client::write_coils_t wr;
    emb::client::write_coil_t wc;

    std::vector<char> to_write, to_read;

    for(i=0; i<1000; ++i) {

        const unsigned int coils_size = (rand() % 0x07B0) + 1;
        const uint16_t coil_address = rand() % ((1 << 16) - coils_size - 1);

        to_write.resize(coils_size);

        for(int j=0; j<coils_size; ++j)
            to_write[j] = (rand() & 1) != 0;

        // Write data
        wr.build_req(coil_address, coils_size, (const bool*)(&to_write[0]));
        res = mb_client.do_transaction(16, 100, wr);
        if(res) printf("Error (write): %d \"%s\"\n", res, emb_strerror(-res));

        const int single_writes = rand() % 10 + 1;
        for(int z=0; z<single_writes; ++z) {
            const bool x = (rand() & 1) != 0;
            const uint16_t pos = rand() % coils_size;
            wc.build_req(coil_address + pos, x);
            res = mb_client.do_transaction(16, 100, wc);
            if(res) printf("Error (read): %d \"%s\"\n", res, emb_strerror(-res));
            to_write[pos] = x;
        }


        // read coils
        rr.build_req(coil_address, coils_size);
        res = mb_client.do_transaction(16, 100, rr);
        if(res) printf("Error (read): %d \"%s\"\n", res, emb_strerror(-res));


        to_read.resize(rr.get_req_quantity());
        rr.response_data((bool*)&to_read[0], to_read.size());

        // Compare
        if(!cmp_boolean_arrays((bool*)&to_write[0], (bool*)&to_read[0], coils_size)) {

            printf("Error: addr:0x%04X, size:0x%04X\n", coil_address, coils_size);

            printf("wrote: ");
            print_boolean_array((bool*)(&to_write[0]), coils_size);
            puts("");
            printf("read:  ");
            print_boolean_array((bool*)(&to_read[0]), coils_size);
            puts("\n");

            ++errors;
        }

        usleep(1000*10);
        //printf("---------------> do_request() := %d\n", res);
    }
    printf("Coils test: errors = %d\n", errors);
}

void registers_test() {
    const uint16_t rr_begin = 0x1000;
    const uint16_t rr_size = 0x1000;
    const uint16_t rr_end = rr_begin + rr_size;

    const int n_tests = 100;
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
/*
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
}*/
