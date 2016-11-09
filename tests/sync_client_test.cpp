
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

#include <emodbus/client/read_coils.h>
#include <emodbus/client/write_coils.h>
#include <emodbus/client/read_regs.h>
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

        event_del(timeout_timer);

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
        //printf("%s:\n", __FUNCTION__); fflush(stdout);
        pthread_mutex_unlock(&mutex);
    }

    void emb_on_error(int _slave_addr, int _errno) {
        is_timeout = false;
        result = _errno;
        //printf("%s:\n", __FUNCTION__); fflush(stdout);
        pthread_mutex_unlock(&mutex);
    }

    static void timeout_cb(evutil_socket_t, short, void * _arg) {
        client_t* _this = (client_t*)_arg;
        _this->is_timeout = true;
        _this->sync_answer_timeout();
        //printf("%s:\n", __FUNCTION__); fflush(stdout);
        pthread_mutex_unlock(&_this->mutex);
    }

    bool is_timeout;

    pthread_mutex_t mutex;
    struct event* timeout_timer;
} mb_client;

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

    //emb_debug_helper.enable_dumping();
    emb_tcp_via_tcp_client_get_transport(rtu)->flags |= EMB_TRANSPORT_FLAG_DUMD_PAKETS;

    client->set_transport(emb_tcp_via_tcp_client_get_transport(rtu));

    client->init_timers(base);

    event_base_dispatch(base);

    emb_tcp_via_tcp_client_destroy(rtu);
}

void print_all_read_file_answer_data(const emb::client::read_file_t &ans);
void write_and_read_file_record_test();
void coils_test();
void registers_test();

void full_test();

int main(int argc, char* argv[]) {

    printf("emodbus sync client test\n");

    srand(time(0));

    pthread_t pthr;

    pthread_create(&pthr, NULL, thr_proc, (void*)&mb_client);

    sleep(1);

    full_test();

   // pthread_join(pthr, NULL);

    printf("RX bytes = %ld\n", emb_debug_helper.rx_bytes());
    printf("TX bytes = %ld\n", emb_debug_helper.tx_bytes());

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

class coils_tester_t {
public:

    typedef std::pair<uint16_t,uint16_t> coils_range_t;

    ~coils_tester_t() {
        for(int i=0; i<servers_size; ++i) {
            delete [] coils[i];
        }
    }

    void initialize(int _servers_begin,
                    int _servers_size) {

        servers_begin = _servers_begin;
        servers_size = _servers_size;

        servers_end = servers_begin + servers_size;

        timeout = 1000;

        tr.ans.resize(MAX_PDU_DATA_SIZE);
        tr.req.resize(MAX_PDU_DATA_SIZE);

        coils.resize(servers_size);
        for(int i=0; i<servers_size; ++i) {
            coils[i] = new bool[0x10000];
        }

        count_0x01 = 0;
        count_0x05 = 0;
        count_0x0F = 0;
    }

    uint16_t rand16() const {
        return rand()+rand();
    }

    void server_coils_read(int _srv_addr) {

        emb::client::read_coils_t rc(tr);

        for(int r=0; r<coils_ranges.size(); ++r) {
            const uint16_t rr_begin = coils_ranges[r].first;
            const uint16_t rr_size = coils_ranges[r].second;
            uint32_t count = 0;
            do {
                const uint16_t begin_addr = rr_begin+count;
                int res, to_read = rr_size-count;
                if(to_read > EMB_READ_COILS_MAX_QUANTITY)
                    to_read = EMB_READ_COILS_MAX_QUANTITY;

                rc.build_req(begin_addr, to_read);

                res = mb_client.do_transaction(_srv_addr, timeout, rc);
                if(res) {
                    printf("%s:%d: Error: %d \"%s\"\n", __FUNCTION__, __LINE__, res, emb_strerror(-res));
                    printf("srv_addr=%d, begin_addr=0x%04X, quantity=0x%04X\n", _srv_addr, begin_addr, to_read);
                    fflush(stdout);
                }

                rc.response_data(coils[_srv_addr-servers_begin] + begin_addr, to_read);

                count += to_read;
            }
            while(count < rr_size);
        }
    }

    void coils_read() {
        for(int i=servers_begin; i<servers_end; ++i) {
            server_coils_read(i);
        }
    }

    void schedule_0x01() {
        int err;

        int srv_addr;
        unsigned int begin_addr;
        unsigned int quantity;

        bool must_be_error;

        srv_addr = srv_addr = servers_begin + (rand() % servers_size);
        begin_addr = rand16();
        quantity = (rand() % EMB_READ_COILS_MAX_QUANTITY) + 1;

//        if(quantity+begin_addr >= 0x10000) {
//            quantity = 0x10000 - begin_addr;
//        }

        if(is_range_belongs_t_regs(begin_addr, quantity)) {
            must_be_error = false;
        }
        else {
            must_be_error = true;
        }

        emb::client::read_coils_t rc(tr);

        uint16_t* p_data;

        rc.build_req(begin_addr, quantity);

        err = mb_client.do_transaction(srv_addr, timeout, rc);

        if((must_be_error && !err) || (!must_be_error && err)) {
            printf("%s:%d: Error: %d \"%s\"\n", __FUNCTION__, __LINE__, err, emb_strerror(-err));
            printf("must_be_error=%d srv_addr=%d, begin_addr=0x%04X, quantity=0x%04X\n",
                   must_be_error, srv_addr, begin_addr, quantity);
            fflush(stdout);
            //exit(0);
        }
        else if(!err) {
            //p_data = &coils[srv_addr-servers_begin][begin_addr];
            //rc.get_answer_regs(p_data, 0, rc.get_answer_quantity());
            rc.response_data(coils[srv_addr-servers_begin] + begin_addr, rc.get_req_quantity());
        }

        ++count_0x01;
    }

    void schedule_0x05() {
        int err;
        const int srv_addr = servers_begin + (rand() % servers_size);

        int addr;

        bool must_be_error;

        addr = rand16();

        if(is_address_belongs_to_coils(addr)) {
            must_be_error = false;
        }
        else {
            must_be_error = true;
        }

        emb::client::write_coil_t wc(tr);

        const bool data = (rand() & 1);

        wc.build_req(addr, data);

        err = mb_client.do_transaction(srv_addr, timeout, wc);
        if((must_be_error && !err) || (!must_be_error && err)) {
            printf("%s:%d: Error: %d \"%s\"\n", __FUNCTION__, __LINE__, err, emb_strerror(-err));
            printf("must_be_error=%d srv_addr=%d, addr=0x%04X,\n",
                   must_be_error, srv_addr, addr);
            fflush(stdout);
            //exit(0);
        }
        else if(!err) {
            coils[srv_addr-servers_begin][addr] = data;
        }

        ++count_0x05;
    }

    void schedule_0x0F() {
        int err;
        const int srv_addr = servers_begin + (rand() % servers_size);

        unsigned int begin_addr;
        unsigned int quantity;

        bool must_be_error;

        begin_addr = rand16();
        quantity = (rand() % EMB_WRITE_COILS_MAX_QUANTITY) + 1;

//        if(quantity+begin_addr > 0x10000) {
//            quantity = 0x10000 - begin_addr;
//        }

        if(is_range_belongs_t_regs(begin_addr, quantity)) {
            must_be_error = false;
        }
        else {
            must_be_error = true;
        }

        emb::client::write_coils_t wc(tr);

        bool* wcoils = new bool[quantity];

        for(int i=0; i<quantity; ++i)
            wcoils[i] = (rand() & 1);


        wc.build_req(begin_addr, quantity, wcoils);

        err = mb_client.do_transaction(srv_addr, timeout, wc);

        if((must_be_error && !err) || (!must_be_error && err)) {
            printf("%s:%d: Error: %d \"%s\"\n", __FUNCTION__, __LINE__, err, emb_strerror(-err));
            printf("must_be_error=%d srv_addr=%d, begin_addr=0x%04X, quantity=0x%04X\n",
                   must_be_error, srv_addr, begin_addr, quantity);
            fflush(stdout);
            //exit(0);
        }
        else if(!err) {
            bool* p_data = coils[srv_addr-servers_begin] + begin_addr;
            //printf("MBE=%d ADDR=0x%04X, Q=0x%04X\n", must_be_error, begin_addr, quantity);
            for(int i=0; i<quantity; ++i) {
                p_data[i] = wcoils[i];
            }
        }

        delete [] wcoils;

        ++count_0x0F;
    }

    void server_range_verify(int _srv_addr) {
        emb::client::read_coils_t rc(tr);

        for(int r=0; r<coils_ranges.size(); ++r) {
            const uint16_t rr_begin = coils_ranges[r].first;
            const uint16_t rr_size = coils_ranges[r].second;
            uint32_t count = 0;
            do {
                const uint16_t begin_addr = rr_begin+count;
                int res, to_read = rr_size-count;
                if(to_read > EMB_READ_COILS_MAX_QUANTITY)
                    to_read = EMB_READ_COILS_MAX_QUANTITY;

                rc.build_req(begin_addr, to_read);

                res = mb_client.do_transaction(_srv_addr, timeout, rc);
                if(res) {
                    printf("%s:%d: Error: %d \"%s\"\n", __FUNCTION__, __LINE__, res, emb_strerror(-res));
                    printf("srv_addr=%d, begin_addr=0x%04X, quantity=0x%04X\n", _srv_addr, begin_addr, to_read);
                    fflush(stdout);
                }

                bool flag = false;

                for(int i=0; i<to_read; ++i) {
                    const uint16_t addr = begin_addr+i;
                    const bool v1 = rc.get_answer_coil(i);
                    const bool v2 = coils[_srv_addr-servers_begin][addr];
                    if(v1 != v2) {
                        printf("%s:%d: server[0x%02X] verify error [0x%04X] %d != %d\n",
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
    }

    void range_verify() {
        for(int i=servers_begin; i<servers_end; ++i) {
            server_range_verify(i);
        }
    }

    bool is_address_belongs_to_coils(uint16_t _addr) const {
        for(int r=0; r<coils_ranges.size(); ++r) {
            const coils_range_t& rr = coils_ranges[r];
            if(rr.first <= _addr && _addr < rr.first+rr.second)
                return true;
        }
        return false;
    }

    bool is_range_belongs_t_regs(uint16_t _addr, uint16_t _size) {
        for(int r=0; r<coils_ranges.size(); ++r) {
            const coils_range_t& rr = coils_ranges[r];
            const uint32_t end = rr.first+rr.second;
            if(rr.first <= _addr && (_addr+_size) <= end)
                return true;
        }
        return false;
    }

    std::vector<coils_range_t> coils_ranges;

    int servers_begin;
    int servers_size;
    int servers_end;

    int timeout;

    emb::client::transaction_t tr;

    std::vector< bool* > coils;

    int count_0x01;
    int count_0x05;
    int count_0x0F;
};

class holdings_tester_t {
public:

    typedef std::pair<uint16_t,uint16_t> reg_range_t;

    void initialize(int _servers_begin,
                    int _servers_size) {

        servers_begin = _servers_begin;
        servers_size = _servers_size;

        servers_end = servers_begin + servers_size;

        timeout = 1000;

        tr.ans.resize(MAX_PDU_DATA_SIZE);
        tr.req.resize(MAX_PDU_DATA_SIZE);

        holdings.resize(servers_size);
        for(int i=0; i<servers_size; ++i) {
            holdings[i].resize(0x10000);
        }

        count_0x03 = 0;
        count_0x06 = 0;
        count_0x10 = 0;
        count_0x16 = 0;
    }

    uint16_t rand16() const {
        return rand()+rand();
    }

    void server_holdings_read(int _srv_addr) {

        emb::client::read_regs_t rr(tr);

        for(int r=0; r<reg_ranges.size(); ++r) {
            const uint16_t rr_begin = reg_ranges[r].first;
            const uint16_t rr_size = reg_ranges[r].second;
            uint32_t count = 0;
            do {
                const uint16_t begin_addr = rr_begin+count;
                int res, to_read = rr_size-count;
                if(to_read > 125)
                    to_read = 125;

                rr.build_req(EMB_RR_HOLDINGS, begin_addr, to_read);

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
    }

    void holdings_read() {
        for(int i=servers_begin; i<servers_end; ++i) {
            server_holdings_read(i);
        }
    }

    void schedule_0x03() {
        int err;

        int srv_addr;
        unsigned int begin_addr;
        unsigned int quantity;

        bool must_be_error;

        srv_addr = srv_addr = servers_begin + (rand() % servers_size);
        begin_addr = rand16();
        quantity = (rand() % 125) + 1;

//        if(quantity+begin_addr >= 0x10000) {
//            quantity = 0x10000 - begin_addr;
//        }

        if(is_range_belongs_t_regs(begin_addr, quantity)) {
            must_be_error = false;
        }
        else {
            must_be_error = true;
        }

        emb::client::read_regs_t rr(tr);

        uint16_t* p_data;

        rr.build_req(EMB_RR_HOLDINGS, begin_addr, quantity);

        err = mb_client.do_transaction(srv_addr, timeout, rr);

        if((must_be_error && !err) || (!must_be_error && err)) {
            printf("%s:%d: Error: %d \"%s\"\n", __FUNCTION__, __LINE__, err, emb_strerror(-err));
            printf("must_be_error=%d srv_addr=%d, begin_addr=0x%04X, quantity=0x%04X\n",
                   must_be_error, srv_addr, begin_addr, quantity);
            fflush(stdout);
            //exit(0);
        }
        else if(!err) {
            p_data = &holdings[srv_addr-servers_begin][begin_addr];
            rr.get_answer_regs(p_data, 0, rr.get_answer_quantity());
        }

        ++count_0x03;
    }

    void schedule_0x06() {
        int err;
        const int srv_addr = servers_begin + (rand() % servers_size);

        int addr;

        bool must_be_error;

        addr = rand16();

        if(is_address_belongs_to_regs(addr)) {
            must_be_error = false;
        }
        else {
            must_be_error = true;
        }

        emb::client::write_reg_t wr(tr);

        const uint16_t data = rand();

        wr.build_req(addr, data);

        err = mb_client.do_transaction(srv_addr, timeout, wr);
        if((must_be_error && !err) || (!must_be_error && err)) {
            printf("%s:%d: Error: %d \"%s\"\n", __FUNCTION__, __LINE__, err, emb_strerror(-err));
            printf("must_be_error=%d srv_addr=%d, addr=0x%04X,\n",
                   must_be_error, srv_addr, addr);
            fflush(stdout);
            //exit(0);
        }
        else if(!err) {
            holdings[srv_addr-servers_begin][addr] = data;
        }

        ++count_0x06;
    }

    void schedule_0x10() {
        int err;
        const int srv_addr = servers_begin + (rand() % servers_size);

        unsigned int begin_addr;
        unsigned int quantity;

        bool must_be_error;

        begin_addr = rand16();
        quantity = (rand() % 123) + 1;

//        if(quantity+begin_addr > 0x10000) {
//            quantity = 0x10000 - begin_addr;
//        }

        if(is_range_belongs_t_regs(begin_addr, quantity)) {
            must_be_error = false;
        }
        else {
            must_be_error = true;
            }

        emb::client::write_regs_t wr(tr);

        emb::regs_t regs;
        regs.resize(quantity);

        for(int i=0; i<quantity; ++i)
            regs[i] = rand();


        wr.build_req(begin_addr, quantity, regs.data());

        err = mb_client.do_transaction(srv_addr, timeout, wr);

        if((must_be_error && !err) || (!must_be_error && err)) {
            printf("%s:%d: Error: %d \"%s\"\n", __FUNCTION__, __LINE__, err, emb_strerror(-err));
            printf("must_be_error=%d srv_addr=%d, begin_addr=0x%04X, quantity=0x%04X\n",
                   must_be_error, srv_addr, begin_addr, quantity);
            fflush(stdout);
            //exit(0);
        }
        else if(!err) {
            uint16_t* p_data = &holdings[srv_addr-servers_begin][begin_addr];
            //printf("MBE=%d ADDR=0x%04X, Q=0x%04X\n", must_be_error, begin_addr, quantity);
            for(int i=0; i<quantity; ++i) {
                p_data[i] = regs[i];
            }
        }

        ++count_0x10;
    }

    void schedule_0x16() {
        int err;
        const int srv_addr = servers_begin + (rand() % servers_size);
        int addr;

        bool must_be_error;

        addr = rand16();

        if(is_address_belongs_to_regs(addr)) {
            must_be_error = false;
        }
        else {
            must_be_error = true;
        }

        emb::client::write_mask_reg_t wm(tr);

        const uint16_t and_mask = rand();
        const uint16_t or_mask = rand();

        uint16_t data = holdings[srv_addr-servers_begin][addr];

        data = (data & and_mask) | (or_mask & ~and_mask);

        wm.build_req(addr, and_mask, or_mask);

        err = mb_client.do_transaction(srv_addr, timeout, wm);
        if((must_be_error && !err) || (!must_be_error && err)) {
            printf("%s:%d: Error: %d \"%s\"\n", __FUNCTION__, __LINE__, err, emb_strerror(-err));
            printf("must_be_error=%d srv_addr=%d, addr=0x%04X and_mask=0x%04X, or_mask=0x%04X\n",
                   must_be_error, srv_addr, addr, and_mask, or_mask);
            fflush(stdout);
            //exit(0);
        }
        else if(!err) {
            holdings[srv_addr-servers_begin][addr] = data;
        }

        ++count_0x16;
    }

    void server_range_verify(int _srv_addr) {
        emb::client::read_regs_t rr(tr);

        for(int r=0; r<reg_ranges.size(); ++r) {
            const uint16_t rr_begin = reg_ranges[r].first;
            const uint16_t rr_size = reg_ranges[r].second;
            uint32_t count = 0;
            do {
                const uint16_t begin_addr = rr_begin+count;
                int res, to_read = rr_size-count;
                if(to_read > 125)
                    to_read = 125;

                rr.build_req(EMB_RR_HOLDINGS, begin_addr, to_read);

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
                        //flag=true;
                    }
                }

                if(flag)
                    exit(0);

                count += to_read;
            }
            while(count < rr_size);
        }
    }

    void range_verify() {
        for(int i=servers_begin; i<servers_end; ++i) {
            server_range_verify(i);
        }
    }

    bool is_address_belongs_to_regs(uint16_t _addr) const {
        for(int r=0; r<reg_ranges.size(); ++r) {
            const reg_range_t& rr = reg_ranges[r];
            if(rr.first <= _addr && _addr < rr.first+rr.second)
                return true;
        }
        return false;
    }

    bool is_range_belongs_t_regs(uint16_t _addr, uint16_t _size) {
        for(int r=0; r<reg_ranges.size(); ++r) {
            const reg_range_t& rr = reg_ranges[r];
            const uint32_t end = rr.first+rr.second;
            if(rr.first <= _addr && (_addr+_size) <= end)
                return true;
        }
        return false;
    }

    std::vector<reg_range_t> reg_ranges;

    int servers_begin;
    int servers_size;
    int servers_end;

    int timeout;

    emb::client::transaction_t tr;

    std::vector< std::vector< uint16_t > > holdings;

    int count_0x03;
    int count_0x06;
    int count_0x10;
    int count_0x16;
};

void full_test() {
    holdings_tester_t ht;
    coils_tester_t ct;

    printf("Begining of the test\n");

    ht.initialize(1, 1);
    ct.initialize(1, 1);

    ht.reg_ranges.push_back(holdings_tester_t::reg_range_t(0x0000, 0x7FED));
    ht.reg_ranges.push_back(holdings_tester_t::reg_range_t(0x7FED, 0x0077));
    ht.reg_ranges.push_back(holdings_tester_t::reg_range_t(0x8065, 0x0001));
    ht.reg_ranges.push_back(holdings_tester_t::reg_range_t(0xE800, 0x1800));

    ct.coils_ranges.push_back(coils_tester_t::coils_range_t(0x0000, 0x7FED));
    ct.coils_ranges.push_back(coils_tester_t::coils_range_t(0x7FED, 0x0077));
    ct.coils_ranges.push_back(coils_tester_t::coils_range_t(0x8065, 0x0001));
    ct.coils_ranges.push_back(coils_tester_t::coils_range_t(0xE800, 0x1800));

    printf("Reading all ..."); fflush(stdout);

    ht.holdings_read();
    ct.coils_read();

    printf("OK\ntesting ... \n"); fflush(stdout);

    enum { N_TESTS = 65536 * 2 * 2 };

    sleep(2);

    for(int i=0; i<N_TESTS; ++i) {

        const unsigned int x = rand()+rand();

        if(x & (1 << 0)) ct.schedule_0x01();
        if(x & (1 << 1)) ct.schedule_0x05();
        if(x & (1 << 2)) ct.schedule_0x0F();

        if(x & (1 << 3)) ht.schedule_0x03();
        if(x & (1 << 4)) ht.schedule_0x06();
        if(x & (1 << 5)) ht.schedule_0x10();
        if(x & (1 << 6)) ht.schedule_0x16();

        if(!(i%(N_TESTS/100))) {
            printf("\rProgress: %d%%", i/(N_TESTS/100));
            fflush(stdout);
        }
    }

    sleep(2);

    printf("\nOK\nVerifying ..."); fflush(stdout);

    ht.range_verify();
    ct.range_verify();

    printf("OK\n"); fflush(stdout);

    printf("count 0x01 calls = %d\n", ct.count_0x01);
    printf("count 0x05 calls = %d\n", ct.count_0x05);
    printf("count 0x0F calls = %d\n", ct.count_0x0F);

    printf("count 0x03 calls = %d\n", ht.count_0x03);
    printf("count 0x06 calls = %d\n", ht.count_0x06);
    printf("count 0x10 calls = %d\n", ht.count_0x10);
    printf("count 0x16 calls = %d\n", ht.count_0x16);
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
