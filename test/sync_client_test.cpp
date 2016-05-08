
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

#include <streams/stream.h>

#include <pthread.h>

#include <event2/event.h>

#include "timespec_operations.h"

//#include "posix_serial_rtu/posix_serial_rtu.hpp"
#include "rtu/rtu.hpp"
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

    rtu_t psp(base, "/dev/ttyUSB0", 115200);
    //rtu_t psp(base, 4003, "192.168.1.163");

    client->set_proto(psp.get_proto());

    psp.get_proto()->flags |= EMB_PROTO_FLAG_DUMD_PAKETS;

    emb_debug_helper.enable_dumping();

    event_base_dispatch(base);
}

void print_all_read_file_answer_data(const emb::client::read_file_t &ans);
void write_and_read_file_record_test();
void coils_test();
void registers_test();

int main(int argc, char* argv[]) {

    printf("emodbus sync client test\n");

    pthread_t pthr;

    pthread_create(&pthr, NULL, thr_proc, (void*)&mb_client);

    sleep(1);

    emb::client::proxy_t d8_proxy(&mb_client, 48);

    d8_proxy.set_timeout(1000);

    for(int i=0; i<100; ++i) {

        try {

            printf("v = 0x%04X\n", (int)d8_proxy.holdings[0]);
            d8_proxy.holdings[0] = 0xC0FE;

            emb::regs_t r = d8_proxy.holdings[emb::range_t(0x40, 0x47)];
            for(int j=0; j<r.size(); ++j)
                printf("0x%04X ", r[j]);
            printf("\n");
        }
        catch (int err) {
            fprintf(stderr, "Error: %s\n", emb_strerror(-err));
        }

        usleep(1000 * 100);
    }

   // pthread_join(pthr, NULL);

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

void write_and_read_file_record_test() {

    using namespace emb;
    using namespace emb::client;

    int res;

    write_file_t wf;
    read_file_t rf;

    // make a write request
    write_file_t::req_t wf_data;
    wf_data << write_file_t::subreq_t(0, 0, regs_t() << 0x1111 << 0xDEAD << 0xC0FE)
            << write_file_t::subreq_t(0, 4, regs_t() << 0x2222 << 0xDEAD << 0xC0FE << 0xABBA)
            << write_file_t::subreq_t(0, 8, regs_t() << 0x3333 << 0xDEAD << 0xF00D);
    wf.build_req(wf_data);

    // make a read request
    read_file_t::req_t rf_data;
    rf_data << read_file_t::subreq_t(0, 0, 3)
            << read_file_t::subreq_t(0, 4, 4)
            << read_file_t::subreq_t(0, 8, 3)
            << read_file_t::subreq_t(0, 0, 20);
    rf.build_req(rf_data);

    // before write
    res = mb_client.do_transaction(16, 100, wf);
    if(res)
        printf("Error: %d \"%s\"\n", res, emb_strerror(-res));

    // after read
    res = mb_client.do_transaction(16, 100, rf);
    if(res)
        printf("Error: %d \"%s\"\n", res, emb_strerror(-res));

    // Iterate over all received data, and print it.
    for(read_file_t::answer_iterator_t ii=rf.subanswer_begin(); ii != rf.subanswer_end(); ++ii) {

        const int quantity = ii.subanswer_quantity();
        printf("Subanswer:(%d): ", quantity);

        for(int j=0; j<quantity; ++j) {
            printf("%04X ", ii.subanswer_data(j));
        }

        printf("\n");
    }

}
