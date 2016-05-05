
#ifndef EMODBUS_HPP
#define EMODBUS_HPP

#include <emodbus/client/client.h>
#include <vector>
#include <list>
#include <emodbus/client/read_file_record.h>
#include <emodbus/client/write_file_record.h>

#include <emodbus/server/server.h>
#include <emodbus/server/coils.h>
#include <emodbus/server/holdings.h>
#include <emodbus/server/file.h>

namespace emb {

// Just overload the << operator for useful usage.
template<typename T>
struct vector : public std::vector<T> {
    vector& operator << (const T& _v) {
        std::vector<T>::push_back(_v);
        return *this;
    }
};

typedef std::pair<uint16_t, uint16_t> range_t;

typedef vector<bool> coils_t;
typedef coils_t inputs_t;

struct regs_t : public std::vector<uint16_t> {
    regs_t& operator << (const int& _v);
    regs_t& operator << (const float& _v);

    regs_t& operator >> (int& _v);
    regs_t& operator >> (float& _v);
//    template <typename X>
//    regs_t& operator << (const X& _v) {
//        const uint16_t* p = (uint16_t*)&_v;
//        for(int i=0; i<(sizeof(X)/2); ++i) {
//            std::vector<uint16_t>::push_back(*p++);
//        }
//        return *this;
//    }
};

// *******************************************************************************
// pdu_t

class pdu_t : public emb_pdu_t {
public:
    pdu_t();
    pdu_t(unsigned int _sz);

    void resize(unsigned int _size);
    operator emb_pdu_t* ();
    operator emb_const_pdu_t* () const;

private:
    std::vector<char> buffer;
};

namespace client {

// *******************************************************************************
//  #####  #       ###   #####  #   #  #####
//  #      #        #    #      ##  #    #
//  #      #        #    #####  # # #    #
//  #      #        #    #      #  ##    #
//  #####  #####   ###   #####  #   #    #

// *******************************************************************************
// transaction_t

class transaction_t {
public:

    transaction_t();

    virtual void emb_transaction_on_response(int _slave_addr);
    virtual void emb_transaction_on_error(int _slave_addr, int _errno);

    pdu_t req, ans;

    operator struct emb_client_transaction_t* ();

private:
    static void emb_trans_on_response_(struct emb_client_transaction_t* _tr,
                                       int _slave_addr);
    static void emb_trans_on_error_(struct emb_client_transaction_t* _tr,
                                    int _slave_addr, int _errno);

    static struct emb_client_req_procs_t procs;
    struct emb_client_transaction_t tr;
};

// *******************************************************************************
// read_coils_t

class read_coils_t : public transaction_t {
public:
    read_coils_t();

    void build_req(uint16_t _starting_address, uint16_t _quantity);

    uint16_t get_req_starting_addr() const;
    uint16_t get_req_quantity() const;
    char get_answer_coil(uint16_t _offset) const;
    uint8_t get_answer_byte(uint8_t _offset) const;

    void response_data(bool* _coils, unsigned int _size) const;
};

// *******************************************************************************
// write_coil_t

class write_coil_t : public transaction_t {
public:
    write_coil_t();

    void build_req(uint16_t _address, bool _value);

    uint16_t get_req_addr() const;
};

// *******************************************************************************
// write_coils_t

class write_coils_t : public transaction_t {
public:
    write_coils_t();

    void build_req(uint16_t _starting_address, uint16_t _quantity,
                   const bool *_pcoils);

    uint16_t get_req_starting_addr() const;
    uint16_t get_req_quantity() const;
};

// *******************************************************************************
// read_regs_t

class read_regs_t : public transaction_t {
public:
    read_regs_t();

    void build_req(uint16_t _starting_address, uint16_t _quantity);

    uint16_t get_req_starting_addr() const;
    uint16_t get_req_quantity() const;
    uint16_t get_answer_reg(uint16_t _offset) const;
    uint16_t get_answer_quantity() const;

    void get_answer_regs(uint16_t* _p_data, uint16_t _offset, uint16_t _n_regs);
    void get_answer_regs(regs_t& _res, uint16_t _offset, uint16_t _n_regs);
};

// *******************************************************************************
// write_reg_t

class write_reg_t : public transaction_t {
public:
    write_reg_t();

    void build_req(uint16_t _address, uint16_t _value);

    uint16_t get_req_address() const;
    uint16_t get_req_value() const;

    uint16_t get_answer_address() const;
    uint16_t get_answer_value() const;
};

// *******************************************************************************
// write_regs_t

class write_regs_t : public transaction_t {
public:
    write_regs_t();

    void build_req(uint16_t _address, uint16_t _quantity, const uint16_t* _data);

    uint16_t get_req_address() const;
    uint16_t get_req_quantity() const;
    uint16_t get_req_data(uint16_t _offset) const;

    uint16_t get_answer_address() const;
    uint16_t get_answer_quantity() const;
};

// *******************************************************************************
// write_mask_reg_t

class write_mask_reg_t : public transaction_t {
public:
    write_mask_reg_t();

    void build_req(uint16_t _address,
                   uint16_t _and_mask,
                   uint16_t _or_mask);

    uint16_t get_req_address() const;
    uint16_t get_req_and_mask() const;
    uint16_t get_req_or_mask() const;

    uint16_t get_answer_address() const;
    uint16_t get_answer_and_mask() const;
    uint16_t get_answer_or_mask() const;
};

// *******************************************************************************
// read_file_t

class read_file_t : public transaction_t {
public:

    // Read file record for
    struct subreq_t : public emb_read_file_req_t {
        subreq_t(uint16_t _fileno, uint16_t _record_no, uint16_t _length);
    };

    typedef vector<subreq_t> req_t;

    struct subanswer_t {
        uint16_t length;
        emb::regs_t data;
    };

    typedef vector<subanswer_t> answer_t;

    class answer_iterator_t {
        friend class read_file_t;
    private:
        answer_iterator_t(emb_const_pdu_t* _anw);

    public:
        uint16_t subanswer_quantity() const;
        uint16_t subanswer_data(int _i) const;

        bool operator == (const answer_iterator_t& _a) const;
        bool operator != (const answer_iterator_t& _a) const;

        void operator ++ ();
        void operator ++ (int);

        void reset_to_begin();

    private:
        emb_const_pdu_t* ans;
        emb_read_file_subansw_t* iterator_;
    };

    read_file_t();

    void build_req(const req_t& _reqs);

    answer_iterator_t subanswer_begin() const;
    answer_iterator_t subanswer_end() const;

    answer_iterator_t operator [] (int _subanswer_index) const;

    answer_t& get_answer(answer_t& _res);
};

// *******************************************************************************
// write_file_t

class write_file_t : public transaction_t {
public:

    struct subreq_t {
        subreq_t(uint16_t _file_number,
                 uint16_t _record_number,
                 const regs_t& _regs);

        uint16_t file_number;
        uint16_t record_number;
        const regs_t regs;
    };

    typedef vector<subreq_t> req_t;

    write_file_t();

    void build_req(const req_t& _req);
};

// *******************************************************************************
// read_fifo_t

class read_fifo_t : public transaction_t {
public:
    read_fifo_t();

    void build_req(uint16_t _starting_address);

    uint16_t get_answer_regs_count() const;
    uint16_t get_answer_data(uint16_t _offset) const;
    uint16_t get_answer_all_data(uint16_t _buf_size, uint16_t* _buf) const;
    regs_t& get_answer_all_data(regs_t& _result) const;
};

// *******************************************************************************
// client_t

class client_t {
public:
    client_t();

    int do_transaction(int _server_addr,
                       unsigned int _timeout,
                       transaction_t& _transaction);

    void set_proto(struct emb_protocol_t* _proto);

    void set_sync(bool _is_sync);
    bool get_sync() const;

    virtual void emb_on_response(int _slave_addr);
    virtual void emb_on_error(int _slave_addr, int _errno);

protected: // (interface for sync mode)
    virtual int emb_sync_client_start_wait(unsigned int _timeout);
    void sync_answer_timeout();

private:
    static void emb_on_response_(struct emb_client_t* _req, int _slave_addr);
    static void emb_on_error_(struct emb_client_t* _req, int _slave_addr, int _errno);

protected:
    struct emb_client_t client;
    transaction_t* curr_transaction;
    int curr_server_addr;
    bool is_sync;
    int result;
};

// *******************************************************************************
// proxy_t

class proxy_t {

    struct holdings_t {
        friend class proxy_t;
    private:
        struct reg_t {
            friend class holdings_t;

            operator int();
            void operator = (int _v);
            void operator |= (int _v);
            void operator &= (int _v);
            void operator = (const emb::regs_t& _regs);

            operator float();
            void operator = (float _v);

            void set_bit(unsigned char _nbit, bool _value);

        private:
            uint16_t addr;
            proxy_t* p;
        } reg;

        struct regs_t {
            friend class holdings_t;

            operator emb::regs_t();
            void operator = (const emb::regs_t& _regs);

        private:
            uint16_t start, end;
            proxy_t* p;
        } regs;

    public:
        reg_t& operator[] (uint16_t _i);
        regs_t& operator[] (const emb::range_t& _r);

    private:
        proxy_t* p;
    };

public:

    enum { DEFAULT_TIMEOUT = 100 };

    proxy_t();

    proxy_t(client_t* _client, int _server_addr);

    void set_client(client_t* _client);
    client_t* client() const;

    void set_server_address(int _server_address);
    int server_address() const;

    void set_timeout(unsigned int _time);
    unsigned int timeout() const;

    // Discrete inputs
    void read_discrete_inputs(uint16_t _begin, uint16_t _size, inputs_t& _result);

    // Coils
    void read_coils(uint16_t _begin, uint16_t _size, coils_t& _result);
    void write_coil(uint16_t _addr, bool _value);
    void write_coils(uint16_t _begin, const coils_t& _values);

    // Input registers
    void read_input_regs(uint16_t _begin, uint16_t _size, regs_t& _result);

    // Holding registers
    void read_holdings(uint16_t _begin, uint16_t _size, regs_t& _result);
    void write_holding(uint16_t _addr, uint16_t _value);
    void write_holdings(uint16_t _begin, const regs_t& _values);
    void read_write_holdings(uint16_t _read_begin, uint16_t _read_size, regs_t& _read_result,
                             uint16_t _write_begin, const regs_t& _to_write);

    // File records
    void read_file(const read_file_t::req_t& _req, read_file_t::answer_t& _result);
    void write_file(const write_file_t::req_t& _req);

    // FIFO
    void read_fifo(uint16_t _begin, regs_t& _result);

public:
    void do_transaction(transaction_t& _tr);

public:
    holdings_t holdings;

private:
    client_t* client_;
    int server_addr_;
    unsigned int timeout_;
};

} // namespace client

namespace server {

// *******************************************************************************
//  #####  #####  ####   #   #  #####  ####
//  #      #      #   #  #   #  #      #   #
//  #####  #####  ####   #   #  #####  ####
//      #  #      #   #   # #   #      #   #
//  #####  #####  #   #    #    #####  #   #

class server_t;

// *******************************************************************************
// coils_t

class coils_t {
    friend class server_t;
public:
    coils_t();
    coils_t(uint16_t _start, uint16_t _size);

    void set_start(uint16_t _start);
    void set_size(uint16_t _size);

    virtual uint8_t on_read_coils(uint16_t _offset,
                                  uint16_t _quantity,
                                  uint8_t* _pvalues);

    virtual uint8_t on_write_coils(uint16_t _offset,
                                   uint16_t _quantity,
                                   const uint8_t* _pvalues);
private:
    void set_funcs();
    static uint8_t read_coils(struct emb_srv_coils_t* _coils,
                              uint16_t _offset,
                              uint16_t _quantity,
                              uint8_t* _pvalues);
    static uint8_t write_coils(struct emb_srv_coils_t* _coils,
                               uint16_t _offset,
                               uint16_t _quantity,
                               const uint8_t* _pvalues);

    struct emb_srv_coils_t coils;
};

// *******************************************************************************
// holdings_t

class holdings_t {
    friend class server_t;
public:
    holdings_t();
    holdings_t(uint16_t _start, uint16_t _size);

    void set_start(uint16_t _start);
    void set_size(uint16_t _size);
    virtual uint8_t on_read_regs(uint16_t _offset,
                                 uint16_t _quantity,
                                 uint16_t* _pvalues);
    virtual uint8_t on_write_regs(uint16_t _offset,
                                  uint16_t _quantity,
                                  const uint16_t* _pvalues);

private:
    void set_funcs();
    static uint8_t read_regs(struct emb_srv_holdings_t* _rr,
                             uint16_t _offset,
                             uint16_t _quantity,
                             uint16_t* _pvalues);
    static uint8_t write_regs(struct emb_srv_holdings_t* _rr,
                              uint16_t _offset,
                              uint16_t _quantity,
                              const uint16_t* _pvalues);

    struct emb_srv_holdings_t h;
};

// *******************************************************************************
// file_record_t

class file_record_t {
    friend class server_t;
public:
    file_record_t();
    file_record_t(uint16_t _fileno/*, uint16_t _start, uint16_t _size*/);

    void set_fileno(uint16_t _fileno);
//    void set_start(uint16_t _start);
//    void set_size(uint16_t _size);
    virtual uint8_t on_read_file(uint16_t _offset,
                                 uint16_t _quantity,
                                 uint16_t* _pvalues);
    virtual uint8_t on_write_file(uint16_t _offset,
                                  uint16_t _quantity,
                                  const uint16_t* _pvalues);

private:
    void set_funcs();
    static uint8_t read_file(struct emb_srv_file_t* _f,
                             uint16_t _offset,
                             uint16_t _quantity,
                             uint16_t* _pvalues);
    static uint8_t write_file(struct emb_srv_file_t* _f,
                              uint16_t _offset,
                              uint16_t _quantity,
                              const uint16_t* _pvalues);

    struct emb_srv_file_t f;
};

// *******************************************************************************
// server_t

class super_server_t;

class server_t {
    friend class super_server_t;

    struct function_t {
        uint8_t func_no;
        emb_srv_function_t func;
    };

public:
    server_t(int _address);

    virtual int addr() const;

    bool add_function(uint8_t _func_no, emb_srv_function_t _func);

    bool add_coils(coils_t& _coils);
    bool add_holdings(holdings_t& _holdings);
    bool add_file(file_record_t& _file);

private:
    static emb_srv_function_t get_function(struct emb_server_t* _srv, uint8_t _func);

    static struct emb_srv_coils_t* get_coils(struct emb_server_t* _srv, uint16_t _begin);

    static struct emb_srv_holdings_t* get_holdings(struct emb_server_t* _srv, uint16_t _begin);

    static struct emb_srv_file_t* get_file(struct emb_server_t* _srv, uint16_t _fileno/*, uint16_t _begin*/);

    typedef std::vector<function_t>::iterator func_iter;
    std::vector<function_t> functions;

    typedef std::vector<coils_t*>::iterator coils_iter;
    std::vector<coils_t*> coils;

    typedef std::vector<holdings_t*>::iterator holdnigs_iter;
    std::vector<holdings_t*> holdings;

    typedef std::vector<file_record_t*>::iterator files_iter;
    std::vector<file_record_t*> files;

protected:
    int address;

private:
    struct emb_server_t srv;
};

// *******************************************************************************
// super_server_t

class super_server_t {
public:
    super_server_t();
    void set_proto(struct emb_protocol_t* _proto);

    bool add_server(server_t& _srv);

private:
    static struct emb_server_t* _get_server(struct emb_super_server_t* _ssrv,
                                            uint8_t _address);
    typedef std::vector<server_t*>::iterator srv_iter;
    std::vector<server_t*> servers;

    struct emb_super_server_t ssrv;
    emb::pdu_t rx_pdu,tx_pdu;
};

} // namespace server

} // namespace emb

#endif // EMODBUS_HPP
