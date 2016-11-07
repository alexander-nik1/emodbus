
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <errno.h>
#include <stdlib.h>

#include <emodbus/emodbus.hpp>

#include <emodbus/client/client.h>
#include <emodbus/base/common.h>
#include <emodbus/transport/rtu.h>
#include <emodbus/base/add/container_of.h>
#include <emodbus/base/modbus_errno.h>


#include <emodbus/client/read_holding_regs.h>
#include <emodbus/client/write_mask_reg.h>
#include <emodbus/client/write_multi_regs.h>
#include <emodbus/client/write_single_reg.h>
#include <emodbus/client/read_file_record.h>
#include <emodbus/client/write_file_record.h>
#include <emodbus/client/read_fifo.h>

#include <emodbus/impl/posix/serial-rtu.hpp>
#include <emodbus/impl/posix/tcp-client-rtu.hpp>
#include <emodbus/impl/posix/mb-tcp-via-tcp-client.h>

#include <pthread.h>

#include <event2/event.h>

#include "timespec_operations.h"

#include "dumping_helper.hpp"

class client_t : public emb::client::client_t {
public:
    client_t()
        : timeout_timer(NULL)
    {
        pthread_mutex_init(&mutex, NULL);
        pthread_mutex_trylock(&mutex);

        emb::client::client_t::set_sync(true);
    }

    ~client_t() {
        if(timeout_timer) {
            event_free(timeout_timer);
            timeout_timer = NULL;
        }
    }

    void init_timers(struct event_base* _eb) {
        timeout_timer = event_new(_eb, -1, EV_TIMEOUT, timeout_cb, this);
    }

    int do_transaction(int _server_addr,
                                 unsigned int _timeout,
                                 emb::client::transaction_t &_transaction) {
        int r;
        while((r = emb::client::client_t::do_transaction(_server_addr, _timeout, _transaction)) == -16) {
            usleep(1);
        }
        return r;
    }

private:

    int emb_sync_client_start_wait(unsigned int _timeout) {
        int res;
        is_timeout = false;

        struct timeval timeout_time;

        timeout_time.tv_sec = 0;
        timeout_time.tv_usec = _timeout * 1000;

        if(timeout_timer) {
            event_add(timeout_timer, &timeout_time);
        }
        else {
            fprintf(stderr, "%s: can't keep timeouts, timeout_timer is NULL!\n", __PRETTY_FUNCTION__);
            return -1;
        }

//        pthread_mutex_trylock(&mutex);

        res = pthread_mutex_lock(&mutex);

        if(is_timeout) {
            return 1;
        }
        else {
            return result;
        }
    }

    void emb_on_response(int _slave_addr) {
        is_timeout = false;
        result = 0;
        pthread_mutex_unlock(&mutex);
    }

    void emb_on_error(int _slave_addr, int _errno) {
        is_timeout = false;
        result = _errno;
        pthread_mutex_unlock(&mutex);
    }

    static void timeout_cb(evutil_socket_t, short, void * _arg) {
        client_t* _this = (client_t*)_arg;
        _this->is_timeout = true;
        _this->sync_answer_timeout();
        pthread_mutex_unlock(&_this->mutex);
    }

    bool is_timeout;

    pthread_mutex_t mutex;
    struct event* timeout_timer;
} mb_client;

emb_debug_helper_t emb_debug_helper;

//serial_rtu_t rtu;
//tcp_client_rtu_t rtu;
//tcp_client_tcp_t rtu;

#include <event2/thread.h>

void* thr_proc(void* p) {

    int res=0;

    client_t* client = (client_t*)p;

    evthread_use_pthreads();

    struct event_base *base = event_base_new();

    struct emb_tcp_via_tcp_client_t* rtu;

    rtu = emb_tcp_via_tcp_client_create(base, "127.0.0.1", 8502);

    //res = rtu.open(base, "/dev/ttyUSB0", 115200);
    //res = rtu.open(base, "127.0.0.1", 8502);

    if(res)
        exit(res);

    emb_debug_helper.enable_dumping();
    //emb_tcp_via_tcp_client_get_transport(rtu)->flags |= EMB_TRANSPORT_FLAG_DUMD_PAKETS;

    client->set_transport(emb_tcp_via_tcp_client_get_transport(rtu));

    client->init_timers(base);

    event_base_dispatch(base);

    emb_tcp_via_tcp_client_destroy(rtu);
}

void print_all_read_file_answer_data(const emb::client::read_file_t &ans);
void write_and_read_file_record_test();
void coils_test();
void registers_test();

void holdings_test();

int main(int argc, char* argv[]) {

    printf("emodbus sync client test\n");

    srand(time(0));

    pthread_t pthr;

    pthread_create(&pthr, NULL, thr_proc, (void*)&mb_client);

    sleep(1);

    holdings_test();

    //coils_test();

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

    const int slave_id = 1;
    const int timeout = 1000;

    // 124 for ruby server.
    const int max_coils_size_per_req = 0x07B0;

    std::vector<char> to_write, to_read;

    for(i=0; i<10000; ++i) {

        const unsigned int coils_size = (rand() % max_coils_size_per_req) + 1;
        const uint16_t coil_address = rand() % ((1 << 16) - coils_size - 1);

        to_write.resize(coils_size);

        for(int j=0; j<coils_size; ++j)
            to_write[j] = (rand() & 1) != 0;

        // Write data
        wr.build_req(coil_address, coils_size, (const bool*)(&to_write[0]));
        res = mb_client.do_transaction(slave_id, timeout, wr);
        if(res) printf("Error (write): %d \"%s\"\n", res, emb_strerror(-res)), fflush(stdout);

        const int single_writes = rand() % 10 + 1;
        for(int z=0; z<single_writes; ++z) {
            const bool x = (rand() & 1) != 0;
            const uint16_t pos = rand() % coils_size;
            wc.build_req(coil_address + pos, x);
            res = mb_client.do_transaction(slave_id, timeout, wc);
            if(res) printf("Error (read): %d \"%s\"\n", res, emb_strerror(-res)), fflush(stdout);
            to_write[pos] = x;
        }


        // read coils
        rr.build_req(coil_address, coils_size);
        res = mb_client.do_transaction(slave_id, timeout, rr);
        if(res) printf("Error (read): %d \"%s\"\n", res, emb_strerror(-res)), fflush(stdout);


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

            fflush(stdout);

            ++errors;
        }

        //usleep(1000*10);
        //printf("---------------> do_request() := %d\n", res);
    }



    printf("Coils test: errors = %d\n", errors);
}


class holdings_tester_t {
public:

    void initialize(uint16_t _rr_begin,
                    uint32_t _rr_size,
                    int _servers_begin,
                    int _servers_size) {

        rr_begin = _rr_begin;
        rr_size = _rr_size;

        servers_begin = _servers_begin;
        servers_size = _servers_size;

        rr_end = rr_begin + rr_size;
        servers_end = servers_begin + servers_size;

        timeout = 1000;

        tr.ans.resize(MAX_PDU_DATA_SIZE);
        tr.req.resize(MAX_PDU_DATA_SIZE);

        holdings.resize(servers_size);
        for(int i=0; i<servers_size; ++i) {
            holdings[i].resize(0x10000);
        }
    }

    void server_range_read(int _srv_addr) {
        uint32_t count = 0;

        emb::client::read_regs_t rr(tr);

        do {
            const uint16_t begin_addr = rr_begin+count;
            int res, to_read = rr_size-count;
            if(to_read > 125)
                to_read = 125;

            rr.build_req(begin_addr, to_read);

            res = mb_client.do_transaction(_srv_addr, timeout, rr);
            if(res) {
                printf("%s:%d: Error: %d \"%s\"\n", __FUNCTION__, __LINE__, res, emb_strerror(-res));
                printf("srv_addr=%d, begin_addr=0x%04X, quantity=0x%04X\n", _srv_addr, begin_addr, to_read);
                fflush(stdout);
            }

            rr.get_answer_regs(&holdings[_srv_addr-servers_begin][begin_addr], 0, to_read);

            count += to_read;
        }
        while(count < rr_size);
    }

    void range_read() {
        for(int i=servers_begin; i<servers_end; ++i) {
            server_range_read(i);
        }
    }

    void schedule_0x03() {
        int res;

        int srv_addr;
        unsigned int begin_addr;
        unsigned int quantity;

        bool must_be_error;

        if(out_of_range_0x03) {
            srv_addr = srv_addr = servers_begin + (rand() % servers_size);
            begin_addr = (rand()*rand()) & 0xFFFF;
            quantity = (rand() % 125) + 1;

            if(quantity+begin_addr >= 0x10000) {
                quantity = 0x10000 - begin_addr;
            }

            if(rr_begin <= begin_addr && begin_addr < rr_end
                && (begin_addr+quantity) <= rr_end) {
                must_be_error = false;
            }
            else {
                must_be_error = true;
            }
        }
        else {
            srv_addr = servers_begin + (rand() % servers_size);
            begin_addr = rr_begin + (rand() % rr_size);
            quantity = (rand() % 125) + 1;

            if(quantity+begin_addr >= rr_end) {
                quantity = rr_end - begin_addr;
            }
            must_be_error = false;
        }

        emb::client::read_regs_t rr(tr);

        uint16_t* p_data;

        rr.build_req(begin_addr, quantity);

        res = mb_client.do_transaction(srv_addr, timeout, rr);

        if((res && !must_be_error) || (!res && must_be_error)) {
            printf("%s:%d: Error: %d \"%s\"\n", __FUNCTION__, __LINE__, res, emb_strerror(-res));
            printf("must_be_error=%d srv_addr=%d, begin_addr=0x%04X, quantity=0x%04X\n",
                   must_be_error, srv_addr, begin_addr, quantity);
            fflush(stdout);
            exit(0);
        }
        else {
            p_data = &holdings[srv_addr-servers_begin][begin_addr];
            rr.get_answer_regs(p_data, 0, rr.get_answer_quantity());
        }
    }

    void schedule_0x06() {
        int res;
        const int srv_addr = servers_begin + (rand() % servers_size);

        int addr;

        bool must_be_error;

        if(out_of_range_0x06) {
            addr = (rand()*rand()) & 0xFFFF;

            if(rr_begin <= addr && addr < rr_end) {
                must_be_error = false;
            }
            else {
                must_be_error = true;
            }
        }
        else {
            addr = rr_begin + (rand() % rr_size);
            must_be_error = false;
        }


        emb::client::write_reg_t wr(tr);

        const uint16_t data = rand();

        wr.build_req(addr, data);

        res = mb_client.do_transaction(srv_addr, timeout, wr);
        if((res && !must_be_error) || (!res && must_be_error)) {
            printf("%s:%d: Error: %d \"%s\"\n", __FUNCTION__, __LINE__, res, emb_strerror(-res));
            printf("must_be_error=%d srv_addr=%d, addr=0x%04X,\n",
                   must_be_error, srv_addr, addr);
            fflush(stdout);
            exit(0);
        }
        else {
            holdings[srv_addr-servers_begin][addr] = data;
        }
    }

    void schedule_0x10() {
        int res;
        const int srv_addr = servers_begin + (rand() % servers_size);

        unsigned int begin_addr;
        unsigned int quantity;

        bool must_be_error;

        if(out_of_range_0x10) {
            begin_addr = (rand()*rand()) & 0xFFFF;
            quantity = (rand() % 123) + 1;

            if(quantity+begin_addr > 0x10000) {
                quantity = 0x10000 - begin_addr;
            }

            if(rr_begin <= begin_addr && begin_addr < rr_end
                && (begin_addr+quantity) <= rr_end) {
                must_be_error = false;
            }
            else {
                must_be_error = true;
            }
        }
        else {
            begin_addr = rr_begin + (rand() % rr_size);
            quantity = (rand() % 123) + 1;

            if(quantity+begin_addr >= rr_end) {
                quantity = rr_end - begin_addr;
            }

            must_be_error = false;
        }

        emb::client::write_regs_t wr(tr);

        emb::regs_t regs;
        regs.resize(quantity);

        for(int i=0; i<quantity; ++i)
            regs[i] = rand();


        wr.build_req(begin_addr, quantity, regs.data());

        res = mb_client.do_transaction(srv_addr, timeout, wr);

        if(must_be_error) {
            if(!res) {
                printf("%s:%d: Error: %d \"%s\"\n", __FUNCTION__, __LINE__, res, emb_strerror(-res));
                printf("must_be_error=%d srv_addr=%d, begin_addr=0x%04X, quantity=0x%04X\n",
                       must_be_error, srv_addr, begin_addr, quantity);
                fflush(stdout);
                exit(0);
            }
        }
        else {
            uint16_t* p_data = &holdings[srv_addr-servers_begin][begin_addr];
            //printf("MBE=%d ADDR=0x%04X, Q=0x%04X\n", must_be_error, begin_addr, quantity);
            for(int i=0; i<quantity; ++i) {
                p_data[i] = regs[i];
            }
        }
    }

    void schedule_0x16() {
        int res;
        const int srv_addr = servers_begin + (rand() % servers_size);
        int addr;

        bool must_be_error;

        if(out_of_range_0x16) {
            addr = (rand()*rand()) & 0xFFFF;

            if(rr_begin <= addr && addr < rr_end) {
                must_be_error = false;
            }
            else {
                must_be_error = true;
            }
        }
        else {
            addr = rr_begin + (rand() % rr_size);
        }

        emb::client::write_mask_reg_t wm(tr);

        const uint16_t and_mask = rand();
        const uint16_t or_mask = rand();

        uint16_t data = holdings[srv_addr-servers_begin][addr];

        data = (data & and_mask) | (or_mask & ~and_mask);

        holdings[srv_addr-servers_begin][addr] = data;

        wm.build_req(addr, and_mask, or_mask);

        res = mb_client.do_transaction(srv_addr, timeout, wm);
        if((res && !must_be_error) || (!res && must_be_error)) {
            printf("%s:%d: Error: %d \"%s\"\n", __FUNCTION__, __LINE__, res, emb_strerror(-res));
            printf("must_be_error=%d srv_addr=%d, and_mask=0x%04X, or_mask=0x%04X\n",
                   must_be_error, srv_addr, and_mask, or_mask);
            fflush(stdout);
        }
    }

    void server_range_verify(int _srv_addr) {
        uint32_t count = 0;

        emb::client::read_regs_t rr(tr);

        do {
            const uint16_t begin_addr = rr_begin+count;
            int res, to_read = rr_size-count;
            if(to_read > 125)
                to_read = 125;

            rr.build_req(begin_addr, to_read);

            res = mb_client.do_transaction(_srv_addr, timeout, rr);
            if(res) {
                printf("%s:%d: Error: %d \"%s\"\n", __FUNCTION__, __LINE__, res, emb_strerror(-res));
                printf("srv_addr=%d, begin_addr=0x%04X, quantity=0x%04X\n", _srv_addr, begin_addr, to_read);
                fflush(stdout);
            }

            bool flag = false;

            for(int i=0; i<to_read; ++i) {
                const uint16_t addr = begin_addr+i;
                const uint16_t v1 = rr.get_answer_reg(i);
                const uint16_t v2 = holdings[_srv_addr-servers_begin][addr];
                if(v1 != v2) {
                    printf("%s:%d: server[0x%02X] verify error [0x%04X] 0x%04X != 0x%04X\n",
                                   __FUNCTION__, __LINE__,
                                   _srv_addr, addr, v1, v2), fflush(stdout);
                    flag=true;
                }
            }

            if(flag)
                exit(0);

            count += to_read;
        }
        while(count < rr_size);
    }

    void range_verify() {
        for(int i=servers_begin; i<servers_end; ++i) {
            server_range_verify(i);
        }
    }

    uint16_t rr_begin;
    uint32_t rr_size;
    uint32_t rr_end;

    int servers_begin;
    int servers_size;
    int servers_end;

    int timeout;

    bool out_of_range_0x03;
    bool out_of_range_0x06;
    bool out_of_range_0x10;
    bool out_of_range_0x16;

    emb::client::transaction_t tr;

    std::vector<std::vector< uint16_t > > holdings;
};

void holdings_test() {
    holdings_tester_t ht;

    ht.initialize(0xF000, 0x1000, 16, 1);

    ht.out_of_range_0x03 = true;
    ht.out_of_range_0x06 = true;
    ht.out_of_range_0x10 = true;
    ht.out_of_range_0x16 = true;

    ht.range_read();

    for(int i=0; i<10000; ++i) {
        ht.schedule_0x03();
        ht.schedule_0x06();
        ht.schedule_0x10();
        ht.schedule_0x16();
    }

    ht.range_verify();
}

void write_and_read_file_record_test()
{

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
